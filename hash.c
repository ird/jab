/*
 * hash.c
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

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "hash.h"

int
hash_file_to_string(const char *p_path, char *p_hash_string)
{
	int result = -1;
	int fd = open(p_path, O_RDONLY);
	if (fd == -1)
	{
		result = 0;
	}
	else
	{
		SHA_CTX sha_ctx;
		if (!SHA1_Init(&sha_ctx))
		{
			result = 0;
		}
		else
		{
			while (result)
			{
				const size_t BUFFER_SIZE = 4096;
				unsigned char buffer[BUFFER_SIZE];
				ssize_t read_size = read(fd, (void *)buffer, BUFFER_SIZE);
				if (read_size == -1)
				{
					result = 0;
				}
				else
				{
					if (!SHA1_Update(&sha_ctx, buffer, read_size))
					{
						result = 0;
					}
					else
					{
						if (read_size < BUFFER_SIZE)
						{
							unsigned char sha1[SHA_DIGEST_LENGTH];
							if (!SHA1_Final(sha1, &sha_ctx))
							{
								result = 0;
							}
							else
							{
								int i;
								for (i = 0; i < SHA_DIGEST_LENGTH; i++)
								{
									sprintf(p_hash_string, "%02x", sha1[i]);
									p_hash_string += 2;
								}
								*p_hash_string = 0;
							}
							break;
						}
					}
				}
			}
		}
		close(fd);
	}
	return result;
}

