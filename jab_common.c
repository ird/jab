/*
 * jab_common.c
 *
 *  Created on: 4/03/2014
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
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

#include "jab_common.h"

char *
append_filename_to_path(const char *p_path, const char *p_filename)
{
	char *result = NULL;
	int path_length = strlen(p_path);
	if ((path_length = strlen(p_path)) != 0)
	{
		int filename_length;
		if ((filename_length = strlen(p_filename)) == 0)
		{
			if ((path_length != 1) && (*(p_path + path_length - 1) == '/'))
			{
				path_length--;
			}
			if ((result = malloc(path_length + 1)) != NULL)
			{
				memcpy(result, p_path, path_length);
				*(result + path_length) = 0;
			}
		}
		else
		{
			if (*(p_path + path_length - 1) == '/')
			{
				path_length--;
			}
			if ((result = malloc(path_length + filename_length + 2)) != NULL)
			{
				memcpy(result, p_path, path_length);
				*(result + path_length) = '/';
				memcpy(result + path_length + 1, p_filename, filename_length + 1);
			}
		}
	}
	return result;
}

int
directory_exists(const char *p_path)
{
	int result = -1;
	DIR *dir;
	if ((dir = opendir(p_path)) == NULL) // TODO: Might exist but don't have permission to read? Must check.
	{
		result = 0;
	}
	else
	{
		closedir(dir);
	}
	return result;
}

int
create_directory_if_not_exists(const char *p_path, mode_t p_mode)
{
	int result = -1;
	if (directory_exists(p_path) == 0)
	{
		if (errno != 2)
		{
			result = 0;
		}
		else if (mkdir(p_path, p_mode) == -1)
		{
			result = 0;
		}
	}
	return result;
}

char *
sha1_string_to_path(const char *p_path, const char *p_sha1_string)
{
	char *result = NULL;
	char filename[3];
	*(filename + 2) = 0;
	*filename = *p_sha1_string;
	*(filename + 1) = *(p_sha1_string + 1);
	char *path_one;
	if ((path_one = append_filename_to_path(p_path, filename)) != NULL)
	{
		if (create_directory_if_not_exists(path_one, 0700) == -1)
		{
			*(filename) = *(p_sha1_string + 2);
			*(filename + 1) = *(p_sha1_string + 3);
			char *path_two;
			if ((path_two = append_filename_to_path(path_one, filename)) != NULL)
			{
				if (create_directory_if_not_exists(path_two, 0700) == -1)
				{
					result = append_filename_to_path(path_two, p_sha1_string);
				}
				free(path_two);
			}
		}
		free(path_one);
	}
	return result;
}

