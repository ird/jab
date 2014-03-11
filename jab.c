/*
 * jab.c
 *
 *  Created on: 15/02/2014
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
#include <fcntl.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>

#include "jab_common.h"

int m_uid;

/*
 * Updates the input p_sha1_string with a SHA1 string representation of the input path.
 * Returns -1 on success otherwise returns 0 where the contents of p_sha1_string is not changed.
 */

int
file_to_sha1_string(const char *p_path, char *p_sha1_string)
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
								const char *HEX = "0123456789abcdef";
								int i;
								for (i = 0; i < SHA_DIGEST_LENGTH; i++)
								{
									*p_sha1_string++ = HEX[(sha1[i] & 0xf0) >> 4];
									*p_sha1_string++ = HEX[sha1[i] & 0x0f];
								}
								*p_sha1_string = 0;
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

/*
 * Prints the last errno string value.
 */
void
print_error()
{
	printf("%s\n", strerror(errno));
}

/*
 * Kind of ensures that the p_source_path file exists at p_target_path. If the p_target_path already exists then assumes that it is in fact
 * a copy of p_source_path and returns -1 otherwise if p_target_path does not exist, attempts to copy p_source_path to p_target_path and
 * returns -1 on success or 0 on failure.
 */

int copy_file_if_not_exist(const char *p_source_path, const char *p_target_path)
{
	int result = -1;
	int target_fd;
	int source_fd;
	struct stat target_stat;
	if (lstat(p_target_path, &target_stat) == -1)
	{
		if (errno != ENOENT)
		{
			print_error();
			result = 0;
		}
		else if ((source_fd = open(p_source_path, O_RDONLY)) == -1)
		{
			print_error();
			result = 0;
		}
		else
		{
			if ((target_fd = open(p_target_path, O_CREAT | O_EXCL | O_TRUNC | O_WRONLY, 0700)) == -1)
			{
				print_error();
				result = 0;
			}
			else
			{
				while (result == -1)
				{
					const size_t BUFFER_SIZE = 4096;
					unsigned char buffer[BUFFER_SIZE];
					ssize_t written_size;
					ssize_t read_size;
					if ((read_size = read(source_fd, (void *)buffer, BUFFER_SIZE)) == -1)
					{
						print_error();
						result = 0;
					}
					else if ((written_size = write(target_fd, (void *)buffer, read_size)) < read_size)
					{
						print_error();
						result = 0;
					}
					else if (written_size < read_size) // Actually this might happen over a network. Check for error and retry if not fatal.
					{
						printf("Failed to write the same amount of bytes as read.\n");
						result = 0;
					}
					else if (read_size < BUFFER_SIZE) // This also might happen over a network. Check for error and retry if not fatal.
					{
						break;
					}
				}
				if (close(target_fd) == -1)
				{
					print_error();
					result = 0;
				}
			}
			if (close(source_fd) == -1)
			{
				print_error();
				result = 0;
			}
		}
	}
	return result;
}

int walk(const char *p_source_path, const char *p_target_path, const char *p_dotjab_path, const char *p_backup_path, const char *p_filename)
{
	int result = -1;
	char *source_path;
	if ((source_path = append_filename_to_path(p_source_path, p_filename)) != NULL)
	{
		DIR *dir;
		if ((dir = opendir(source_path)) == NULL)
		{
			print_error();
			result = 0;
		}
		else
		{
			char *backup_path;
			if ((backup_path = append_filename_to_path(p_backup_path, p_filename)) != NULL)
			{
				printf("%s -> %s\n", source_path, backup_path);
				struct stat dir_stat;
				if (lstat(source_path, &dir_stat) == -1)
				{
					print_error();
					result = 0;
				}
				else if (mkdir(backup_path, 0700) == -1)
				{
					print_error();
					result = 0;
				}
				else
				{
					while (result == -1)
					{
						errno = 0;
						struct dirent *dirent;
						if ((dirent = readdir(dir)) == NULL)
						{
							if (errno != 0)
							{
								print_error();
								result = 0;
							}
							else
							{
								break;
							}
						}
						else if (!strcmp(dirent->d_name, ".") || !strcmp(dirent->d_name, ".."))
						{
							continue;
						}
						else
						{
							char *path;
							if ((path = append_filename_to_path(source_path, dirent->d_name)) == NULL)
							{
								result = 0;
							}
							else
							{
								struct stat stat;
								if (lstat(path, &stat) == -1)
								{
									print_error();
								}
								else
								{
									if (S_ISREG(stat.st_mode))
									{
										// TODO: look in previous backup for same filename in same directory.
										// if not exist, sha1 source then find sha1 in .jab otherwise copy
										char sha1_string[SHA_DIGEST_LENGTH * 2 + 1];
										if (file_to_sha1_string(path, sha1_string) == 0)
										{
											printf("Unable to SHA1 %s\n", path);
										}
										else
										{
											printf("%s %s\n", sha1_string, dirent->d_name);
											char *dotjab_filename;
											if ((dotjab_filename = sha1_string_to_path(p_dotjab_path, sha1_string)) == NULL)
											{
												printf("Out of memory.\n");
											}
											else
											{
												printf("%s\n", dotjab_filename);
												if (copy_file_if_not_exist(path, dotjab_filename) == -1)
												{
													int pointer_fd;
													char *pointer_filename;
													if ((pointer_filename = append_filename_to_path(backup_path, dirent->d_name)) == NULL)
													{
														printf("Out of memory.\n");
													}
													else if ((pointer_fd = open(pointer_filename, O_CREAT | O_EXCL | O_TRUNC | O_WRONLY, 0700)) == -1)
													{
														print_error();
													}
													else
													{
														if (write(pointer_fd, sha1_string, SHA_DIGEST_LENGTH * 2 + 1) < SHA_DIGEST_LENGTH * 2 + 1)
														{
															printf("Failed to write pointer file.\n");
														}
														else
														{
															// TODO: write file size after sha1. Or maybe we don't need the file size as we
															// should be able to get that from the .jab SHA1 file.
														}
														if (close(pointer_fd) == -1)
														{
															print_error();
														}
														else
														{
															if (m_uid == 0)
															{
																if (chmod(pointer_filename, stat.st_mode) == -1)
																{
																	printf("Failed to set mode.\n");
																}
																if (chown(pointer_filename, stat.st_uid, stat.st_gid) == -1)
																{
																	printf("Failed to set owner and group.\n");
																}
																// TODO: Set create time also.
																struct utimbuf utimbuf;
																utimbuf.actime = stat.st_atim.tv_sec;
																utimbuf.modtime = stat.st_mtim.tv_sec;
																if (utime(pointer_filename, &utimbuf) == -1)
																{
																	printf("Failed to set times.\n");
																}
															}
															else
															{
															}
															// TODO: if pid 0, set owner, group, permissions and time stamps
															// otherwise set permissions and time stamps.
														}
													}
												}
												free(dotjab_filename);
											}
										}
									}
									else if (S_ISDIR(stat.st_mode))
									{
										walk(source_path, p_target_path, p_dotjab_path, backup_path, dirent->d_name);
									}
									else if (S_ISCHR(stat.st_mode))
									{
									}
									else if (S_ISBLK(stat.st_mode))
									{
									}
									else if (S_ISFIFO(stat.st_mode))
									{
									}
									else if (S_ISLNK(stat.st_mode))
									{
									}
									else if (S_ISSOCK(stat.st_mode))
									{
									}
								}
								free(path);
							}
						}
					}
				}
				free(backup_path);
			}
			closedir(dir);
		}
		free(source_path);
	}
	return result;
}

int
main(int argc, char **argv)
{
	int result = -1;
	if (argc < 4)
	{
		printf("Usage: jab source_path target_path backup_name\n");
		result = 0;
	}
	else
	{
		m_uid = getuid();
		char *source_path = *(argv + 1);
		char *target_path = *(argv + 2);
		char *backup_name = *(argv + 3);
		char *dotjab_path;
		char *backup_path;
		if ((dotjab_path = append_filename_to_path(target_path, ".jab")) == NULL)
		{
			printf("Out of memory.\n");
			result = 0;
		}
		else
		{
			if (directory_exists(source_path) == 0)
			{
				printf("Source path does not exist.\n");
				result = 0;
			}
			else if (create_directory_if_not_exists(target_path, 0700) == 0)
			{
				print_error();
				result = 0;
			}
			else if (create_directory_if_not_exists(dotjab_path, 0700) == 0)
			{
				print_error();
				result = 0;
			}
			else if ((backup_path = append_filename_to_path(target_path, backup_name)) == NULL)
			{
				printf("Out of memory.\n");
				result = 0;
			}
			else
			{
				result = walk(source_path, target_path, dotjab_path, backup_path, "");
				free(backup_path);
			}
			free(dotjab_path);
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

