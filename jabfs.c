/*
 * jabfs.c
 *
 *  Created on: 3/03/2014
 *      Author: Stacey Richards
 *
 * The author disclaims copyright to this source code. In place of a legal notice, here is a blessing:
 *
 *    May you do good and not evil.
 *    May you find forgiveness for yourself and forgive others.
 *    May you share freely, never taking more than you give.
 */

#define FUSE_USE_VERSION 26

#include <errno.h>
#include <dirent.h>
#include <fuse.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <syslog.h>
#include <unistd.h>

#include "jab_common.h"

char *m_source_path;
char *m_target_path;
char *m_dotjab_path;

int
jabfs_getattr(const char *p_path, struct stat *p_stat)
{

	int result = 0;
	char *path;
	if ((path = append_filename_to_path(m_source_path, p_path)) == NULL)
	{
		result = -ENOMEM;
	}
	else
	{
		if (lstat(path, p_stat) == -1)
		{
			result = -errno;
		}
		else if (S_ISREG(p_stat->st_mode))
		{
			int fd;
			if ((fd = open(path, O_RDONLY)) == -1)
			{
				result = -errno;
			}
			else
			{
				char sha1[41];
				int bytes_read;
				if ((bytes_read = read(fd, sha1, 41)) == -1)
				{
					result = -errno;
				}
				else if (bytes_read < 41)
				{
					result = -EPERM;
				}
				else
				{
					char *real_path;
					if ((real_path = sha1_string_to_path(m_dotjab_path, sha1)) == NULL)
					{
						result = -ENOMEM;
					}
					else
					{
						struct stat stat;
						lstat(real_path, &stat);
						p_stat->st_size = stat.st_size;
						free(real_path);
					}
				}
				close(fd);
			}
		}
		free(path);
	}
	return result;
}

int
jabfs_readdir(const char *p_path, void *p_buffer, fuse_fill_dir_t p_fuse_fill_dir, off_t p_offset, struct fuse_file_info *p_fuse_file_info)
{
	int result = 0;
	char *path;
	if ((path = append_filename_to_path(m_source_path, p_path)) == NULL)
	{
		result = -ENOMEM;
	}
	else
	{
		DIR *dir;
		if ((dir = opendir(path)) == NULL)
		{
			result = -errno;
		}
		else
		{
			while (result == 0)
			{
				errno = 0;
				struct dirent *dirent;
				if ((dirent = readdir(dir)) == NULL)
				{
					if (errno != 0)
					{
						result = -errno;
					}
					else
					{
						break;
					}
				}
				else if (strcmp(p_path, "/") != 0 || strcmp(dirent->d_name, ".jab") != 0)
				{
					if (p_fuse_fill_dir(p_buffer, dirent->d_name, NULL, 0) != 0)
					{
						result = -ENOMEM;
					}
				}
			}
			closedir(dir);
		}
		free(path);
	}
	return result;
}

int
jabfs_open(const char *p_path, struct fuse_file_info *p_fuse_file_info)
{
	int result = 0;
	char *path;
	if ((path = append_filename_to_path(m_source_path, p_path)) == NULL)
	{
		result = -ENOMEM;
	}
	else
	{
		int fd;
		if ((fd = open(path, O_RDONLY)) == -1)
		{
			result = -errno;
		}
		else
		{
			char sha1[41];
			int bytes_read;
			if ((bytes_read = read(fd, sha1, 41)) == -1)
			{
				result = -errno;
			}
			else if (bytes_read < 41)
			{
				result = -EPERM;
			}
			else
			{
				char *real_path;
				if ((real_path = sha1_string_to_path(m_dotjab_path, sha1)) == NULL)
				{
					result = -ENOMEM;
				}
				else
				{
					int fh;
					if ((fh = open(real_path, p_fuse_file_info->flags)) == -1)
					{
						result = -errno;
					}
					else
					{
						p_fuse_file_info->fh = fh;
					}
					free(real_path);
				}
			}
			close(fd);
		}
		free(path);
	}
	return result;
}

int
jabfs_read(const char *p_path, char *p_buffer, size_t p_size, off_t p_offset, struct fuse_file_info *p_fuse_file_info)
{
	int result;
	if ((result = pread(p_fuse_file_info->fh, p_buffer, p_size, p_offset)) == -1)
	{
		result = -errno;
	}
	return result;
}

int
jabfs_release(const char *p_path, struct fuse_file_info *p_fuse_file_info)
{
	int result;
    if ((result = close(p_fuse_file_info->fh)) == -1)
    {
    	result = -errno;
    }
    return result;
}

struct fuse_operations fuse_ops =
{
	.getattr = jabfs_getattr,
	.readdir = jabfs_readdir,
	.open = jabfs_open,
	.read = jabfs_read,
	.release = jabfs_release
};

int
main(int p_argc, char **p_argv)
{
	int result = -1;
	if (p_argc < 3)
	{
		printf("Usage: jabfs source_path target_path\n");
		result = 0;
	}
	else
	{
		if ((m_source_path = append_filename_to_path(*(p_argv + 1), "")) == NULL)
		{
			printf("Out of memory.\n");
			result = 0;
		}
		else
		{
			if ((m_target_path = append_filename_to_path(*(p_argv + 2), "")) == NULL)
			{
				printf("Out of memory\n");
				result = 0;
			}
			else
			{
				if ((m_dotjab_path = append_filename_to_path(m_source_path, ".jab")) == NULL)
				{
					printf("Out of memory\n");
					result = 0;
				}
				else
				{
					if (directory_exists(m_source_path) == 0)
					{
						printf("Source path does not exist.\n");
						result = 0;
					}
					else if (directory_exists(m_target_path) == 0)
					{
						printf("Target path does not exist.\n");
						result = 0;
					}
					else if (directory_exists(m_dotjab_path) == 0)
					{
						printf("Source is not a JAB path.\n");
						result = 0;
					}
					else
					{
						*(p_argv + 1) = *(p_argv + 2);
						*(p_argv + 2) = NULL;
						if (fuse_main(p_argc - 1, p_argv, &fuse_ops, NULL) != 0)
						{
							printf("Mount error.\n");
							result = 0;
						}
					}
					free(m_dotjab_path);
				}
				free(m_target_path);
			}
			free(m_source_path);
		}
	}
	if (result)
	{
		return EXIT_SUCCESS;
	}
	else
	{
		return EXIT_FAILURE;
	}
}

