/*
 * backup.c
 *
 *  Created on: 21/09/2013
 *      Author: Stacey Richards
 *
 * The author disclaims copyright to this source code. In place of a legal notice, here is a blessing:
 *
 *    May you do good and not evil.
 *    May you find forgiveness for yourself and forgive others.
 *    May you share freely, never taking more than you give.
 */

#include <stdio.h>
#include <stdlib.h>

#include "database.h"
#include "source.h"
#include "target.h"

/*
 * ISSUES
 * ======
 * Target filenames with spaces aren't begin created correctly. Example "this is a file.txt" is being created as "this".
 * source_process_dirent lstat Value too large for defined data type
 *
 */
int
main(int argc, char **argv)
{
	int result = -1;
	if (argc < 4)
	{
		printf("Usage: backup [source path] [target path] [backup name]\n");
		result = 0;
	}
	else
	{
		if (!target_setup(*(argv + 2), *(argv + 3)))
		{
			result = 0;
		}
		else
		{
			if (!database_open())
			{
				result = 0;
			}
			else if (!database_begin_transaction())
			{
				result = 0;
			}
			else if (!source(*(argv + 1)))
			{
				database_rollback_transaction();
				result = 0;
			}
			else
			{
				database_commit_transaction();
				printf("Directories found:   %lu\n", m_source_directories_found);
				printf("Directories skipped: %lu\n", m_source_directories_skipped);
				printf("Files found:         %lu\n", m_source_files_found);
				printf("Files skipped:       %lu\n", m_source_files_skipped);
			}
			database_close();
			target_cleanup();
		}
	}
	if (result)
	{
		exit(0);
	}
	else
	{
		exit(1);
	}
}
