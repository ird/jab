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
 * When compiling on the Raspberry Pi, one may have to add -D_FILE_OFFSET_BITS=64.
 * http://code.google.com/p/libarchive/issues/detail?id=318
 *
 * Database calls here should probably be shifted to target.
 *
 * At this stage, when starting a backup, the named directory should not currently exist. This means that a partial backup can not be completed by specifying
 * the same name. There is some code still floating around that is expecting a "completion run". This should probably be cleaned out and then once everything
 * else is working as expected, we can introduce partial backup completion later.
 */
int
main(int argc, char **argv)
{
	int result = -1;
	if (argc < 4)
	{
		printf("Usage: backup source_path target_path backup_name\n");
		result = 0;
	}
	else
	{
		if (!database_open(*(argv + 2)))
		{
			result = 0;
		}
		else if (!database_begin_transaction())
		{
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
				if (!source(*(argv + 1)))
				{
					result = 0;
				}
				else
				{
					if (!target_set_last())
					{
						result = 0;
					}
					else
					{
						printf("Directories found:   %lu\n", m_source_directories_found);
						printf("Directories skipped: %lu\n", m_source_directories_skipped);
						printf("Files found:         %lu\n", m_source_files_found);
						printf("Files skipped:       %lu\n", m_source_files_skipped);
					}
				}
				target_cleanup();
			}
			if (result)
			{
				database_commit_transaction();
			}
			else
			{
				database_rollback_transaction();
			}
		}
		database_close();
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
