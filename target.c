/*
 * target.c
 *
 *  Created on: 22/09/2013
 *      Author: Stacey Richards
 *
 * The author disclaims copyright to this source code. In place of a legal notice, here is a blessing:
 *
 *    May you do good and not evil.
 *    May you find forgiveness for yourself and forgive others.
 *    May you share freely, never taking more than you give.
 */

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "database.h"
#include "transfer.h"

char *m_target_base_path;

size_t m_target_base_path_size;

char *m_target_current_path;

size_t m_target_current_path_size;

struct stat m_target_current_stat;

struct tm m_target_current_tm;

struct dirent m_target_current_dirent;

off_t m_target_remaining_size;

FILE *m_target_file;

char *
target_target_base_path_plus_path(char *p_path)
{
	char *target_base_path_plus_path = NULL;
	size_t path_size = strlen(p_path);
	target_base_path_plus_path = (char *)malloc(m_target_base_path_size + path_size + 1);
	if (target_base_path_plus_path != NULL)
	{
		memcpy(target_base_path_plus_path, m_target_base_path, m_target_base_path_size);
		memcpy(target_base_path_plus_path + m_target_base_path_size, p_path, path_size + 1);
	}
	return target_base_path_plus_path;
}

int
target_setup_target_base_path(char *p_path)
{
	int result = -1;
	size_t path_size = strlen(p_path);
	if (path_size && *(p_path + path_size - 1) != '/')
	{
		if ((m_target_base_path = (char *)malloc(path_size + 2)) == NULL)
		{
			result = 0;
		}
		else
		{
			memcpy(m_target_base_path, p_path, path_size);
			*(m_target_base_path + path_size) = '/';
			*(m_target_base_path + path_size + 1) = 0;
		}
	}
	else
	{
		if ((m_target_base_path = (char *)malloc(path_size + 1)) == NULL)
		{
			result = 0;
		}
		else
		{
			memcpy(m_target_base_path, p_path, path_size + 1);
		}
	}
	return result;
}

int
target_setup(char *p_path)
{
	int result = -1;
	if (!target_setup_target_base_path(p_path))
	{
		printf("target target_setup_base_path\n");
		result = 0;
	}
	else
	{
		m_target_base_path_size = strlen(m_target_base_path);
	}
	return result;
}

int
target_cleanup()
{
	int result = -1;
	free(m_target_base_path);
	return result;
}

int
target_receive_file()
{
	int result = -1;
	int input_items_matched =
		sscanf
		(
			(char *)m_transfer_buffer,
			"0%o %d %d %lu %04d-%02d-%02d %02d:%02d:%02d %s\n",
			&m_target_current_stat.st_mode,
			&m_target_current_stat.st_uid,
			&m_target_current_stat.st_gid,
			&m_target_current_stat.st_size,
			&m_target_current_tm.tm_year,// + 1900,
			&m_target_current_tm.tm_mon,// + 1,
			&m_target_current_tm.tm_mday,
			&m_target_current_tm.tm_hour,
			&m_target_current_tm.tm_min,
			&m_target_current_tm.tm_sec,
			m_target_current_dirent.d_name
		);
	if (input_items_matched == EOF)
	{
		result = 0;
	}
	else if (input_items_matched < 11)
	{
		result = 0;
	}
	else
	{
		m_target_current_tm.tm_year -= 1900;
		m_target_current_tm.tm_mon--;
	}
	m_target_current_path = target_target_base_path_plus_path(m_target_current_dirent.d_name);
	if (stat(m_target_current_path, &m_target_current_stat) == -1)
	{
		if (errno == 2)
		{
			m_target_file = fopen(m_target_current_path, "w");
			if (m_target_file == NULL)
			{
				printf("target_receive_file fopen\n");
				result = 0;
			}
			else
			{
				m_target_remaining_size = m_target_current_stat.st_size;
				memcpy(m_transfer_buffer, "DATA\n\0", 6);
			}
		}
		else
		{
			printf("target stat error: %d %s\n", errno, strerror(errno));
			result = 0;
		}
	}
	else
	{
		memcpy(m_transfer_buffer, "DONE\n\0", 6);
	}

	//memcpy(m_transfer_buffer, "HASH\n\0", 6);
	//memcpy(m_transfer_buffer, "DATA\n\0", 6);
	//memcpy(m_transfer_buffer, "STOP\n\0", 6);
	return result;
}

int
target_receive_path()
{
	int result = -1;
	return result;
}

int
target_receive_hash()
{
	int result = -1;
	return result;
}

int
target_receive_data()
{
	int result = -1;
	size_t transfer_size = TRANSFER_BUFFER_SIZE;
	if (m_target_remaining_size < transfer_size)
	{
		transfer_size = m_target_remaining_size;
	}
	size_t bytes_written = fwrite(m_transfer_buffer, 1, transfer_size, m_target_file);
	if (bytes_written < transfer_size)
	{
		printf("target_receive_data fwrite\n");
		result = 0;
	}
	else
	{
		m_target_remaining_size -= transfer_size;
		if (m_target_remaining_size == 0)
		{
			fclose(m_target_file);
		}
	}
	return result;
}

int
target_receive_stop()
{
	int result = -1;
	return result;
}
