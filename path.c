/*
 * path.c
 *
 *  Created on: 29/09/2013
 *      Author: Stacey Richards
 *
 * The author disclaims copyright to this source code. In place of a legal notice, here is a blessing:
 *
 *    May you do good and not evil.
 *    May you find forgiveness for yourself and forgive others.
 *    May you share freely, never taking more than you give.
 */

#include <stdlib.h>
#include <string.h>

char *
path_path_plus_name(char *p_path, char *p_name)
{
	char *path_plus_name = NULL;
	size_t path_size = strlen(p_path);
	size_t name_size = strlen(p_name);
	if (!path_size)
	{
		path_plus_name = (char *)malloc(name_size + 1);
		if (path_plus_name != NULL)
		{
			memcpy(path_plus_name, p_name, name_size + 1);
		}
	}
	else
	{
		path_plus_name = (char *)malloc(path_size + 1 + name_size + 1);
		if (path_plus_name != NULL)
		{
			memcpy(path_plus_name, p_path, path_size);
			*(path_plus_name + path_size) = '/';
			memcpy(path_plus_name + path_size +  1, p_name, name_size + 1);
		}
	}
	return path_plus_name;
}

