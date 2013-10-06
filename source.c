/*
 * source.c
 *
 *  Created on: 27/09/2013
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
#include <sys/types.h>
#include <time.h>

#include "hash.h"
#include "path.h"
#include "transfer.h"

unsigned long m_source_directories_found;

unsigned long m_source_directories_skipped;

unsigned long m_source_files_found;

unsigned long m_source_files_skipped;

dev_t m_source_dev;

char *m_source_base_path;

size_t m_source_base_path_size;

int
source_process_path(char *p_path);

int
source_format_file(struct stat *p_stat, struct dirent *p_dirent)
{
	int result = -1;
	struct tm *tm;
	if ((tm = gmtime((time_t *)&p_stat->st_mtim)) == NULL)
	{
		result = 0;
	}
	else
	{
		int size =
			snprintf
			(
				(void *)&m_transfer_buffer,
				TRANSFER_BUFFER_SIZE,
				"0%o %d %d %lu %04d-%02d-%02d %02d:%02d:%02d %s\n",
				p_stat->st_mode,
				p_stat->st_uid,
				p_stat->st_gid,
				p_stat->st_size,
				tm->tm_year + 1900,
				tm->tm_mon + 1,
				tm->tm_mday,
				tm->tm_hour,
				tm->tm_min,
				tm->tm_sec,
				p_dirent->d_name
			);
		if (size < 0)
		{
			result = 0;
		}
		else if (size >= TRANSFER_BUFFER_SIZE)
		{
			result = 0;
		}
	}
	return result;
}
int
source_format_path(struct stat *p_stat, struct dirent *p_dirent)
{
	int result = -1;
	struct tm *tm;
	if ((tm = gmtime((time_t *)&p_stat->st_mtim)) == NULL)
	{
		result = 0;
	}
	else
	{
		printf
		(
			"> 0%o %d %d %04d-%02d-%02d %02d:%02d:%02d %s\n",
			p_stat->st_mode,
			p_stat->st_uid,
			p_stat->st_gid,
			tm->tm_year + 1900,
			tm->tm_mon + 1,
			tm->tm_mday,
			tm->tm_hour,
			tm->tm_min,
			tm->tm_sec,
			p_dirent->d_name
		);
	}
	return result;
}

char *
source_source_base_path_plus_path_plus_name(char *p_path, char *p_name)
{
	char *filename = NULL;
	size_t path_size = strlen(p_path);
	size_t name_size = strlen(p_name);
	if (!path_size)
	{
		filename = (char *)malloc(m_source_base_path_size + name_size + 1);
		if (filename != NULL)
		{
			memcpy(filename, m_source_base_path, m_source_base_path_size);
			memcpy(filename + m_source_base_path_size, p_name, name_size + 1);
		}
	}
	else
	{
		filename = (char *)malloc(m_source_base_path_size + path_size + 1 + name_size + 1);
		if (filename != NULL)
		{
			memcpy(filename, m_source_base_path, m_source_base_path_size);
			memcpy(filename + m_source_base_path_size, p_path, path_size);
			*(filename + m_source_base_path_size + path_size) = '/';
			memcpy(filename + m_source_base_path_size + path_size + 1, p_name, name_size + 1);
		}
	}
	return filename;
}

char *
source_source_base_path_plus_path(char *p_path)
{
	char *path = NULL;
	size_t path_size = strlen(p_path);
	path = (char *)malloc(m_source_base_path_size + path_size + 1);
	if (path != NULL)
	{
		memcpy(path, m_source_base_path, m_source_base_path_size);
		memcpy(path + m_source_base_path_size, p_path, path_size + 1);
	}
	return path;
}

int
source_process_hash(char *p_filename)
{
	int result = -1;
	char sha1_string[SHA1_STRING_SIZE + 1];
	if (!hash_file_to_string(p_filename, sha1_string))
	{
		result = 0;
	}
	else
	{
		transfer_hash_to_target();
		// .
		// .
		// .
	}
	return result;
}

int
source_process_isreg(struct dirent *p_dirent, struct stat *p_stat, char *p_filename)
{
	int result = -1;
	m_source_files_found++;
	if (!source_format_file(p_stat, p_dirent))
	{
		printf("source_process_isreg source_format_file\n");
		result = 0;
	}
	else if (!transfer_file_to_target())
	{
		printf("source_process_isreg transfer_file_to_target\n");
		result = 0;
	}
	else
	{
		if (strcmp((char *)m_transfer_buffer, "DONE\n") != 0)
		{
			if (strcmp((char *)m_transfer_buffer, "HASH\n") == 0)
			{
				if (!source_process_hash(p_filename))
				{
					printf("source_process_isreg source_process_hash\n");
					result = 0;
				}
			}
			else if (strcmp((char *)m_transfer_buffer, "DATA\n") == 0)
			{
				// send data
			}
			else if (strcmp((char *)m_transfer_buffer, "STOP\n") == 0)
			{
				result = 0;
			}
		}
	}
	return result;
}

int
source_process_isdir(char *p_path, struct dirent *p_dirent, struct stat *p_stat)
{
	int result = -1;
	m_source_directories_found++;
	source_format_path(p_stat, p_dirent);
	char *next_path = path_path_plus_name(p_path, p_dirent->d_name);
	if (next_path == NULL)
	{
		printf("source_process_isdir path_path_plus_name\n");
		result = 0;
	}
	else
	{
		result = source_process_path(next_path);
		printf("<\n");
		free(next_path);
	}
	return result;
}

int
source_process_stat(char *p_path, struct dirent *p_dirent, struct stat *p_stat, char *p_filename)
{
	int result = -1;
	if (S_ISLNK(p_stat->st_mode))
	{
		// do nothing yet.
	}
	else if (S_ISREG(p_stat->st_mode))
	{
		if (!source_process_isreg(p_dirent, p_stat, p_filename))
		{
			printf("source_process_stat source_process_isreg %s\n", p_filename);
			result = 0;
		}
	}
	else if (S_ISDIR(p_stat->st_mode))
	{
		if (!source_process_isdir(p_path, p_dirent, p_stat))
		{
			printf("source_process_stat source_process_isdir\n");
			result = 0;
		}
	}
	return result;
}

int
source_process_dirent(char *p_path, struct dirent *p_dirent)
{
	int result = -1;
	char *filename = source_source_base_path_plus_path_plus_name(p_path, p_dirent->d_name);
	if (filename == NULL)
	{
		printf("source_process_dirent source_source_base_path_plus_path_plus_name\n");
		result = 0;
	}
	else
	{
		struct stat stat;
		if (lstat(filename, &stat) == -1)
		{
			printf("source_process_dirent lstat\n");
			result = 0;
		}
		else if (stat.st_dev == m_source_dev && !S_ISFIFO(stat.st_mode) && !S_ISCHR(stat.st_mode) && !S_ISBLK(stat.st_mode) && !S_ISSOCK(stat.st_mode))
		{
			if (!source_process_stat(p_path, p_dirent, &stat, filename))
			{
				printf("source_process_dirent source_process_stat\n");
				result = 0;
			}
		}
		free(filename);
	}
	return result;
}

int
source_process_dir(char *p_path, DIR *p_dir)
{
	int result = -1;
	while (result)
	{
		errno = 0;
		struct dirent *dirent = readdir(p_dir);
		if (dirent == NULL)
		{
			if (errno)
			{
				printf("source_process_dir readdir %s\n", strerror(errno));
				result = 0;
			}
			break;
		}
		if (!strcmp(dirent->d_name, ".") || !strcmp(dirent->d_name, ".."))
		{
			continue;
		}
		if (!source_process_dirent(p_path, dirent))
		{
			printf("source_process_dir source_process_dirent\n");
			result = 0;
		}
	}
	return result;
}

int
source_process_path(char *p_path)
{
	int result = -1;
	char *path = source_source_base_path_plus_path(p_path);
	if (path == NULL)
	{
		printf("source_process_path source_source_base_path_plus_path\n");
		result = 0;
	}
	else
	{
		DIR *dir = opendir(path);
		if (dir == NULL)
		{
			if (errno != EACCES)
			{
				printf("source_process_path opendir %s\n", strerror(errno));
				result = 0;
			}
		}
		else
		{
			if (!source_process_dir(p_path, dir))
			{
				printf("source_process_path source_process_dir\n");
				result = 0;
			}
			closedir(dir);
		}
		free(path);
	}
	return result;
}

int
source_setup_source_base_path(char *p_path)
{
	int result = -1;
	size_t path_size = strlen(p_path);
	if (path_size && *(p_path + path_size - 1) != '/')
	{
		if ((m_source_base_path = (char *)malloc(path_size + 2)) == NULL)
		{
			result = 0;
		}
		else
		{
			memcpy(m_source_base_path, p_path, path_size);
			*(m_source_base_path + path_size) = '/';
			*(m_source_base_path + path_size + 1) = 0;
		}
	}
	else
	{
		if ((m_source_base_path = (char *)malloc(path_size + 1)) == NULL)
		{
			result = 0;
		}
		else
		{
			memcpy(m_source_base_path, p_path, path_size + 1);
		}
	}
	return result;
}

int
source_setup_device(char *p_path)
{
	int result = -1;
	struct stat stat;
	if (lstat(p_path, &stat) == -1)
	{
		result = 0;
	}
	else
	{
		m_source_dev = stat.st_dev;
	}
	return result;
}

int
source(char *p_path)
{
	int result = -1;
	if (!source_setup_device(p_path))
	{
		printf("source source_setup_device\n");
		result = 0;
	}
	else if (!source_setup_source_base_path(p_path))
	{
		printf("source source_setup_base_path\n");
		result = 0;
	}
	else
	{
		m_source_base_path_size = strlen(m_source_base_path);
		m_source_directories_found = 0;
		m_source_directories_skipped = 0;
		m_source_files_found = 0;
		m_source_files_skipped = 0;
		result = source_process_path("");
		free(m_source_base_path);
	}
	return result;
}
