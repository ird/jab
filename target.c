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
#include <unistd.h>

#include "database.h"
#include "hash.h"
#include "transfer.h"

char *m_target_name;

size_t m_target_name_size;

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
 * The current path of our backup target.
 */
char *m_target_current_path;

/*
 * The size in bytes on m_target_current_path not including the trailing 0 string terminator. The size of the memory allocated for m_target_current_path will
 * be m_target_current_path_size + 1.
 */
size_t m_target_current_path_size;

size_t m_target_current_path_offset;

struct stat m_target_current_stat;

struct tm m_target_current_tm;

struct dirent m_target_current_dirent;

char m_target_current_hash[SHA1_STRING_SIZE + 1];

off_t m_target_remaining_size;

FILE *m_target_file;

char *
target_target_base_path_plus_filename(char *p_filename)
{
	char *path = NULL;
	size_t path_size = strlen(p_filename);
	path = (char *)malloc(m_target_base_path_size + path_size + 1);
	if (path != NULL)
	{
		memcpy(path, m_target_base_path, m_target_base_path_size);
		memcpy(path + m_target_base_path_size, p_filename, path_size + 1);
	}
	return path;
}

int
target_enter_directory(char *p_directory_name)
{
	int result = -1;
	size_t size = strlen(p_directory_name);
	char *path = realloc(m_target_current_path, m_target_current_path_size + size + 2);
	if (path == NULL)
	{
		printf("target_append_path_to_target_current_path realloc\n");
		result = 0;
	}
	else
	{
		m_target_current_path = path;
		memcpy(m_target_current_path + m_target_current_path_size, p_directory_name, size);
		*(m_target_current_path + m_target_current_path_size + size) = '/';
		*(m_target_current_path + m_target_current_path_size + size + 1) = 0;
		m_target_current_path_size += size + 1;
	}
	return result;
}

int
target_leave_directory()
{
	int result = -1;
	char *tail = m_target_current_path + m_target_current_path_size - 2;
	while (*tail != '/')
	{
		tail--;
	}
	size_t size = tail - m_target_current_path + 1;
	char *path = realloc(m_target_current_path, size + 1);
	if (path == NULL)
	{
		printf("target_receive_back realloc\n");
		result = 0;
	}
	else
	{
		m_target_current_path = path;
		m_target_current_path_size = size;
		*(m_target_current_path + m_target_current_path_size) = 0;
	}
	return result;
}

int
target_enter_file(char *p_file_name)
{
	int result = -1;
	size_t size = strlen(p_file_name);
	char *path = realloc(m_target_current_path, m_target_current_path_size + size + 2);
	if (path == NULL)
	{
		printf("target_enter_file realloc\n");
		result = 0;
	}
	else
	{
		m_target_current_path = path;
		memcpy(m_target_current_path + m_target_current_path_size, p_file_name, size + 1);
		m_target_current_path_size += size;
	}
	return result;
}

int
target_leave_file()
{
	int result = -1;
	char *tail = m_target_current_path + m_target_current_path_size - 1;
	while (*tail != '/')
	{
		tail--;
	}
	size_t size = tail - m_target_current_path + 1;
	char *path = realloc(m_target_current_path, size + 1);
	if (path == NULL)
	{
		printf("target_leave_file realloc\n");
		result = 0;
	}
	else
	{
		m_target_current_path = path;
		m_target_current_path_size = size;
		*(m_target_current_path + m_target_current_path_size) = 0;
	}
	return result;
}
/*
 * target_setup should probably be called through transfer.c. target_setup will eventually be running on a different machine as source_setup. While
 * target_setup and target_source are running in the one process we'll just call target_setup directly.
 */
int
target_setup(char *p_path, char *p_name)
{
	int result = -1;
	size_t path_size = strlen(p_path);
	if (path_size && *(p_path + path_size - 1) == '/')
	{
		if ((m_target_base_path = (char *)malloc(path_size + 1)) == NULL)
		{
			printf("target_setup malloc m_target_base_path/\n");
			result = 0;
		}
		else
		{
			memcpy(m_target_base_path, p_path, path_size + 1);
			m_target_base_path_size = path_size;
		}
	}
	else
	{
		if ((m_target_base_path = (char *)malloc(path_size + 2)) == NULL)
		{
			printf("target_setup malloc m_target_base_path\n");
			result = 0;
		}
		else
		{
			memcpy(m_target_base_path, p_path, path_size);
			*(m_target_base_path + path_size) = '/';
			*(m_target_base_path + path_size + 1) = 0;
			m_target_base_path_size = path_size + 1;
		}
	}
	if (result)
	{
		struct stat s;
		if (stat(m_target_base_path, &s) == -1)
		{
			if (errno == ENOENT)
			{
				if (mkdir(m_target_base_path, 0700) == -1)
				{
					printf("target_setup mkdir %d %s %s\n", errno, strerror(errno), m_target_base_path);
					result = 0;
				}
			}
			else
			{
				printf("target_setup stat %d %s %s\n", errno, strerror(errno), m_target_base_path);
				result = 0;
			}
		}
		if (result)
		{
			// TODO: name must not be empty or contain a /.
			m_target_name_size = strlen(p_name);
			if ((m_target_name = (char *)malloc(m_target_name_size + 1)) == NULL)
			{
				printf("target_setup malloc m_target_name\n");
				result = 0;
			}
			else
			{
				memcpy(m_target_name, p_name, m_target_name_size + 1);
				if ((m_target_current_path = (char *)malloc(m_target_base_path_size + m_target_name_size + 2)) == NULL)
				{
					printf("target_setup malloc m_target_current_path\n");
					result = 0;
				}
				else
				{
					memcpy(m_target_current_path, m_target_base_path, m_target_base_path_size);
					memcpy(m_target_current_path + m_target_base_path_size, m_target_name, m_target_name_size);
					*(m_target_current_path + m_target_base_path_size + m_target_name_size) = '/';
					*(m_target_current_path + m_target_base_path_size + m_target_name_size + 1) = 0;
					m_target_current_path_size = m_target_base_path_size + m_target_name_size + 1;
					m_target_current_path_offset = m_target_current_path_size;
					if (mkdir(m_target_current_path, 0700) == -1)
					{
						printf("target_setup mkdir %d %s %s\n", errno, strerror(errno), m_target_current_path);
						result = 0;
					}
					if (!result)
					{
						free(m_target_current_path);
					}
				}
				if (!result)
				{
					free(m_target_name);
				}
			}
		}
		if (!result)
		{
			free(m_target_base_path);
		}
	}
	return result;
}

int
target_cleanup()
{
	int result = -1;
	free(m_target_base_path);
	free(m_target_name);
	free(m_target_current_path);
	return result;
}

int
target_receive_file()
{
	int result = -1;
	strcpy(m_target_current_dirent.d_name, (char *)m_transfer_buffer);
	target_enter_file(m_target_current_dirent.d_name);
	struct stat s;
	if (stat(m_target_current_path, &s) == -1)
	{
		if (errno == ENOENT)
		{
			memcpy(m_transfer_buffer, "HASH\n\0", 6);
		}
		else
		{
			printf("target stat error: %d %s\n", errno, strerror(errno));
			result = 0;
		}
	}
	else
	{
		printf("target_receive_file file exists %s\n", m_target_current_path);
		result = 0;
	}
	return result;
}

int
target_receive_path()
{
	int result = -1;
	if (!target_enter_directory((char *)m_transfer_buffer))
	{
		printf("target_receive_path target_append_path_to_target_current_path %s\n", (char *)m_transfer_buffer);
		result = 0;
	}
	else
	{
		struct stat s;
		if (stat(m_target_current_path, &s) == -1)
		{
			if (errno != ENOENT)
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
target_receive_back()
{
	return target_leave_directory();
}

int
target_receive_hash()
{
	int result = -1;
	strcpy(m_target_current_hash, (char *)m_transfer_buffer);
	const unsigned char *name;
	const unsigned char *path;
	sqlite3_stmt *stmt;
	if (!database_prepare("select file.name name, file.path path from file where file.hash = :hash", &stmt))
	{
		result = 0;
	}
	else
	{
		if (!database_bind_text(stmt, ":hash", (const char *)m_transfer_buffer))
		{
			result = 0;
		}
		else
		{
			int rc = sqlite3_step(stmt);
			if (rc == SQLITE_DONE)
			{
				m_target_file = fopen(m_target_current_path, "w");
				if (m_target_file == NULL)
				{
					printf("target_receive_hash fopen %d %s %s\n", errno, strerror(errno), m_target_current_path);
					result = 0;
				}
				else
				{
					m_target_remaining_size = m_target_current_stat.st_size;
					memcpy(m_transfer_buffer, "DATA\n\0", 6);
				}
			}
			else if (rc == SQLITE_ROW)
			{
				if ((name = sqlite3_column_text(stmt, 0)) == NULL)
				{
					result = 0;
				}
				else if ((path = sqlite3_column_text(stmt, 1)) == NULL)
				{
					result = 0;
				}
				else
				{
					size_t name_size = strlen((const char *)name);
					size_t path_size = strlen((const char *)path);
					char *existing_path = malloc(m_target_base_path_size + name_size + 1 + path_size + 1);
					if (existing_path == NULL)
					{
						printf("target_receive_hash malloc\n");
						result = 0;
					}
					else
					{
						memcpy(existing_path, m_target_base_path, m_target_base_path_size);
						memcpy(existing_path + m_target_base_path_size, name, name_size);
						*(existing_path + m_target_base_path_size + name_size) = '/';
						memcpy(existing_path + m_target_base_path_size + name_size + 1, path, path_size + 1);
						if (link(existing_path, m_target_current_path) == -1)
						{
							printf("target_receive_hash link %d %s %s %s\n", errno, strerror(errno), existing_path, m_target_current_path);
							result = 0;
						}
						else if (!target_leave_file())
						{
							printf("target_receive_hash target_leave_file\n");
							result = 0;
						}
						else
						{
							memcpy(m_transfer_buffer, "DONE\n\0", 6);
						}
					}
				}
			}
			else
			{
				result = 0;
			}
		}
		sqlite3_finalize(stmt);
	}
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
	}
	return result;
}

int
target_receive_done()
{
	int result = -1;
	if (fclose(m_target_file) == EOF)
	{
		printf("target_receive_done fclose %d %s %s\n", errno, strerror(errno), m_target_current_path);
		result = 0;
	}
	else
	{
		sqlite3_stmt *stmt;
		if (!database_prepare("insert into file (hash, name, path) values (:hash, :name, :path)", &stmt))
		{
			result = 0;
		}
		else
		{
			if (!database_bind_text(stmt, ":hash", m_target_current_hash))
			{
				result = 0;
			}
			else if (!database_bind_text(stmt, ":name", m_target_name))
			{
				result = 0;
			}
			else if (!database_bind_text(stmt, ":path", m_target_current_path + m_target_current_path_offset))
			{
				result = 0;
			}
			else if (sqlite3_step(stmt) != SQLITE_DONE)
			{
				result = 0;
			}
			sqlite3_finalize(stmt);
		}

		if (!target_leave_file())
		{
			printf("target_receive_done target_leave_file\n");
			result = 0;
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
