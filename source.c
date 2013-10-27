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

char *m_source_current_path;

size_t m_source_current_path_size;

int
source_process_current_path();

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
	/*struct tm *tm;
	if ((tm = gmtime((time_t *)&p_stat->st_mtim)) == NULL)
	{
		result = 0;
	}
	else
	{
		*/int size =
			snprintf
			(
				(void *)&m_transfer_buffer,
				TRANSFER_BUFFER_SIZE,
				/*"0%o %d %d %04d-%02d-%02d %02d:%02d:%02d */"%s\n",
				/*p_stat->st_mode,
				p_stat->st_uid,
				p_stat->st_gid,
				tm->tm_year + 1900,
				tm->tm_mon + 1,
				tm->tm_mday,
				tm->tm_hour,
				tm->tm_min,
				tm->tm_sec,
				*/p_dirent->d_name
			);
		if (size < 0)
		{
			result = 0;
		}
		else if (size >= TRANSFER_BUFFER_SIZE)
		{
			result = 0;
		}
	//}
	return result;
}

char *
source_source_current_path_plus_filename(char *p_filename)
{
	size_t filename_size = strlen(p_filename);
	char *result = malloc(m_source_current_path_size + filename_size + 1);
	if (result == NULL)
	{
		printf("source_source_current_path_plus_filename malloc\n");
	}
	else
	{
		memcpy(result, m_source_current_path, m_source_current_path_size);
		memcpy(result + m_source_current_path_size, p_filename, filename_size + 1);
	}
	return result;
}

int
source_enter_directory(char *p_directory_name)
{
	int result = -1;
	size_t size = strlen(p_directory_name);
	char *path = realloc(m_source_current_path, m_source_current_path_size + size + 2);
	if (path == NULL)
	{
		printf("source_enter_directory realloc\n");
		result = 0;
	}
	else
	{
		m_source_current_path = path;
		memcpy(m_source_current_path + m_source_current_path_size, p_directory_name, size);
		*(m_source_current_path + m_source_current_path_size + size) = '/';
		*(m_source_current_path + m_source_current_path_size + size + 1) = 0;
		m_source_current_path_size += size + 1;
	}
	return result;
}

int
source_leave_directory()
{
	int result = -1;
	char *tail = m_source_current_path + m_source_current_path_size - 2;
	while (*tail != '/')
	{
		tail--;
	}
	size_t size = tail - m_source_current_path + 1;
	char *path = realloc(m_source_current_path, size + 1);
	if (path == NULL)
	{
		printf("source_leave_directory realloc\n");
		result = 0;
	}
	else
	{
		m_source_current_path = path;
		m_source_current_path_size = size;
		*(m_source_current_path + m_source_current_path_size) = 0;
	}
	return result;
}

int
source_enter_file(char *p_file_name)
{
	int result = -1;
	size_t size = strlen(p_file_name);
	char *path = realloc(m_source_current_path, m_source_current_path_size + size + 2);
	if (path == NULL)
	{
		printf("source_enter_file realloc\n");
		result = 0;
	}
	else
	{
		m_source_current_path = path;
		memcpy(m_source_current_path + m_source_current_path_size, p_file_name, size + 1);
		m_source_current_path_size += size;
	}
	return result;
}

int
source_leave_file()
{
	int result = -1;
	char *tail = m_source_current_path + m_source_current_path_size - 1;
	while (*tail != '/')
	{
		tail--;
	}
	size_t size = tail - m_source_current_path + 1;
	char *path = realloc(m_source_current_path, size + 1);
	if (path == NULL)
	{
		printf("source_leave_file realloc\n");
		result = 0;
	}
	else
	{
		m_source_current_path = path;
		m_source_current_path_size = size;
		*(m_source_current_path + m_source_current_path_size) = 0;
	}
	return result;
}

int
source_process_hash(char *p_filename)
{
	int result = -1;
	if (!hash_file_to_string(p_filename, (char *)m_transfer_buffer))
	{
		result = 0;
	}
	return result;
}

int
source_process_isreg(struct dirent *p_dirent, struct stat *p_stat, char *p_filename)
{
	int result = -1;
	m_source_files_found++;
	FILE *file = fopen(p_filename, "r");
	if (errno == EACCES)
	{
		m_source_files_skipped++;
	}
	else if (file == NULL)
	{
		printf("source_process_isreg fopen %d %s %s\n", errno, strerror(errno), p_filename);
		result = 0;
	}
	else
	{
		//printf("          %s\n", p_filename);
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
			if (strcmp((char *)m_transfer_buffer, "DONE\n") == 0)
			{
				printf("source_process_isreg DONE\n");
				result = 0;
			}
			else if (strcmp((char *)m_transfer_buffer, "HASH\n") == 0)
			{
				if (!source_process_hash(p_filename))
				{
					printf("source_process_isreg source_process_hash\n");
					result = 0;
				}
				else
				{
					if (!transfer_hash_to_target())
					{
						printf("source_process_isreg transfer_hash_to_target\n");
						result = 0;
					}
					else if (strcmp((char *)m_transfer_buffer, "DATA\n") == 0)
					{
						off_t remaining_size = p_stat->st_size;
						size_t transfer_size = TRANSFER_BUFFER_SIZE;
						while (remaining_size)
						{
							// TODO: Comparing off_t with size_t?
							if (remaining_size < transfer_size)
							{
								transfer_size = remaining_size;
							}
							size_t bytes_read = fread(m_transfer_buffer, 1, transfer_size, file);
							if (bytes_read < transfer_size)
							{
								printf("source_process_isreg fread\n");
								result = 0;
								break;
							}
							else
							{
								if (!transfer_data_to_target())
								{
									printf("source_process_isreg transfer_data_to_target\n");
									result = 0;
									break;
								}
								else
								{
									remaining_size -= transfer_size;
								}
							}
						}
						if (result)
						{
							if (!transfer_done_to_target())
							{
								printf("source_process_isreg transfer_done_to_target\n");
								result = 0;
							}
						}
					}
				}
			}
			else if (strcmp((char *)m_transfer_buffer, "DATA\n") == 0)
			{
				printf("source_process_isreg DATA\n");
				result = 0;
			}
			else if (strcmp((char *)m_transfer_buffer, "STOP\n") == 0)
			{
				printf("source_process_isreg STOP\n");
				result = 0;
			}
		}
		fclose(file);
	}
	return result;
}

int
source_process_isdir(struct dirent *p_dirent, struct stat *p_stat)
{
	int result = -1;
	m_source_directories_found++;
	strcpy((char *)m_transfer_buffer, p_dirent->d_name);
	if (!transfer_path_to_target())
	{
		printf("source_process_isdir transfer_path_to_target\n");
		result = 0;
	}
	else if (!source_enter_directory(p_dirent->d_name))
	{
		printf("source_process_isdir source_append_path_to_source_current_path\n");
		result = 0;
	}
	else if (!source_process_current_path())
	{
		printf("source_process_isdir source_process_current_path\n");
		result = 0;
	}
	else if (!source_format_path(p_stat, p_dirent))
	{
		printf("source_process_isdir source_format_path\n");
		result = 0;
	}
	else if (!transfer_back_to_target())
	{
		printf("source_process_isdir transfer_back_to_target\n");
		result = 0;
	}
	else if (!source_leave_directory())
	{
		printf("source_process_isdir source_back\n");
		result = 0;
	}
	return result;
}

int
source_process_stat(struct dirent *p_dirent, struct stat *p_stat, char *p_filename)
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
		if (!source_process_isdir(p_dirent, p_stat))
		{
			printf("source_process_stat source_process_isdir\n");
			result = 0;
		}
	}
	return result;
}

int
source_process_dirent(struct dirent *p_dirent)
{
	int result = -1;
	// TODO: Something.
	char *filename = source_source_current_path_plus_filename(p_dirent->d_name);
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
			if (errno != EACCES)
			{
				printf("source_process_dirent lstat %d %s%s \n", errno, strerror(errno), filename);
				result = 0;
			}
		}
		else if (stat.st_dev == m_source_dev && !S_ISFIFO(stat.st_mode) && !S_ISCHR(stat.st_mode) && !S_ISBLK(stat.st_mode) && !S_ISSOCK(stat.st_mode))
		{
			if (!source_process_stat(p_dirent, &stat, filename))
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
source_process_dir(DIR *p_dir)
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
		if (!source_process_dirent(dirent))
		{
			printf("source_process_dir source_process_dirent\n");
			result = 0;
		}
	}
	return result;
}

int
source_process_current_path()
{
	int result = -1;
	DIR *dir = opendir(m_source_current_path);
	if (dir == NULL)
	{
		if (errno != EACCES)
		{
			printf("source_process_current_path opendir %s\n", strerror(errno));
			result = 0;
		}
	}
	else
	{
		if (!source_process_dir(dir))
		{
			printf("source_process_current_path source_process_dir\n");
			result = 0;
		}
		closedir(dir);
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
		if ((m_source_current_path = malloc(m_source_base_path_size + 1)) == NULL)
		{
			printf("source malloc\n");
			result = 0;
		}
		else
		{
			memcpy(m_source_current_path, m_source_base_path, m_source_base_path_size + 1);
			m_source_current_path_size = m_source_base_path_size;
			m_source_directories_found = 0;
			m_source_directories_skipped = 0;
			m_source_files_found = 0;
			m_source_files_skipped = 0;
			result = source_process_current_path();
			free(m_source_current_path);
		}
		free(m_source_base_path);
	}
	return result;
}
