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

/*
 * The base path of our backup target. m_target_base_path always has a trailing /.
 */
char *m_target_base_path;

/*
 * The size in bytes of m_target_base_path not including the trailing 0 string terminator. The size of the memory allocated for m_target_base_path will be
 * m_target_base_path_size + 1.
 */
size_t m_target_base_path_size;

/*
 * The current path of our backup target. m_target_current_path always has a trailing /.
 */
char *m_target_current_path;

/*
 * The size in bytes on m_target_current_path not including the trailing 0 string terminator. The size of the memory allocated for m_target_current_path will
 * be m_target_current_path_size + 1.
 */
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
target_append_path_to_target_current_path(char *p_path)
{
	int result = -1;
	size_t path_size = strlen(p_path);
	char *target_current_path = realloc(m_target_current_path, m_target_current_path_size + path_size + 2);
	if (target_current_path == NULL)
	{
		printf("target_append_path_to_target_current_path realloc\n");
		result = 0;
	}
	else
	{
		m_target_current_path = target_current_path;
		memcpy(m_target_current_path + m_target_current_path_size, p_path, path_size);
		*(m_target_current_path + m_target_current_path_size + path_size) = '/';
		*(m_target_current_path + m_target_current_path_size + path_size + 1) = 0;
		m_target_current_path_size += path_size + 1;
	}
	return result;
}

int
target_append_filename_to_target_current_path(char *p_filename)
{
	int result = -1;
	size_t filename_size = strlen(p_filename);
	char *path = realloc(m_target_current_path, m_target_current_path_size + filename_size + 2);
	if (path == NULL)
	{
		printf("target_append_filename_to_target_current_path realloc\n");
		result = 0;
	}
	else
	{
		m_target_current_path = path;
		memcpy(m_target_current_path + m_target_current_path_size, p_filename, filename_size + 1);
		m_target_current_path_size += filename_size;
	}
	return result;
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
		if ((m_target_current_path = malloc(m_target_base_path_size + 1)) == NULL)
		{
			printf("target_setup malloc\n");
			result = 0;
		}
		else
		{
			memcpy(m_target_current_path, m_target_base_path, m_target_base_path_size + 1);
			m_target_current_path_size = m_target_base_path_size;
		}
	}
	return result;
}

int
target_cleanup()
{
	int result = -1;
	free(m_target_base_path);
	free(m_target_current_path);
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
		printf("target_receive_file sscanf EOF\n");
		result = 0;
	}
	else if (input_items_matched < 11)
	{
		printf("target_receive_file sscanf\n");
		result = 0;
	}
	else
	{
		m_target_current_tm.tm_year -= 1900;
		m_target_current_tm.tm_mon--;
		target_append_filename_to_target_current_path(m_target_current_dirent.d_name);
		struct stat s;
		if (stat(m_target_current_path, &s) == -1) // don't use m_target_current_stat; this will destroy the stat of the transfered file
		{
			if (errno == 2)
			{
				m_target_file = fopen(m_target_current_path, "w");
				if (m_target_file == NULL)
				{
					printf("target_receive_file fopen %d %s %s\n", errno, strerror(errno), m_target_current_path);
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
	if (!target_append_path_to_target_current_path((char *)m_transfer_buffer))
	{
		printf("target_receive_path target_append_path_to_target_current_path %s\n", (char *)m_transfer_buffer);
		result = 0;
	}
	else
	{
		struct stat s;
		if (stat(m_target_current_path, &s) == -1)
		{
			if (errno != 2)
			{
				printf("target_receive_path stat %d %s\n", errno, strerror(errno));
				result = 0;
			}
			else
			{
				/*
				 * The target directory doesn't exist so we'll create it with loose enough permissions to allow us to populate it. When we leave the directory
				 * we'll set the permissions and date/time to the same as the source.
				 */
				if (mkdir(m_target_current_path, 0700) == -1)
				{
					printf("target_receive_path mkdir\n");
					result = 0;
				}
				else
				{
					memcpy(m_transfer_buffer, "DONE\n\0", 6);
				}
			}
		}
		else
		{
			/*
			 * The directory already exists so we don't need to create it. We might have problems populating it if the permissions are too restrictive but
			 * we'll cross that bridge when we come to it.
			 */
			memcpy(m_transfer_buffer, "DONE\n\0", 6);
		}
	}
	return result;
}

int
target_file_done()
{
	int result = -1;
	char *tail = m_target_current_path + m_target_current_path_size - 1;
	while (*tail != '/')
	{
		tail--;
	}
	size_t size = tail - m_target_current_path + 1;
	tail = realloc(m_target_current_path, size + 1);
	if (tail == NULL)
	{
		printf("target_file_done realloc\n");
		result = 0;
	}
	else
	{
		m_target_current_path = tail;
		m_target_current_path_size = size;
		*(m_target_current_path + m_target_current_path_size) = 0;
	}
	return result;
}

int
target_receive_back()
{
	int result = -1;
	char *tail = m_target_current_path + m_target_current_path_size - 2;
	while (*tail != '/')
	{
		tail--;
	}
	size_t size = tail - m_target_current_path + 1;
	tail = realloc(m_target_current_path, size + 1);
	if (tail == NULL)
	{
		printf("target_receive_back realloc\n");
		result = 0;
	}
	else
	{
		m_target_current_path = tail;
		m_target_current_path_size = size;
		*(m_target_current_path + m_target_current_path_size) = 0;
	}
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
			target_file_done();
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
