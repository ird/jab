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
 * At this stage, when starting a backup, the named directory should not currently exist. This means that a partial backup can not be completed by specifying
 * the same name. There is some code still floating around that is expecting a "completion run". This should probably be cleaned out and then once everything
 * else is working as expected, we can introduce partial backup completion later.
 *
 * We still need to set permissions when leaving directories.
 *
 * I'm starting to think that the first stat match block (against the last backup) may not be needed as if the last backup had the file then it will be in the
 * database as either the first instance of the file's hash or a following instance. I need to think about the modified time before removing that code though.
 *
 * There seems to be too many hash matches. Look into this.
 *
 * Still haven't transferred symbolic links. Must get onto this.
 *
 * It seems that all the hard linking to zero length files is messing up the modified time which forces a hash to be sent for all zero length files.
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
		else if (!target_init(*(argv + 2), *(argv + 3)))
		{
			result = 0;
		}
		else
		{
			if (!source(*(argv + 1)))
			{
				result = 0;
			}
			target_free();
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
