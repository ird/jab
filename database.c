/*
 * database.c
 *
 *  Created on: 23/09/2013
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
#include <string.h>

#include "database.h"

#include "configuration.h"

sqlite3 *db = NULL;

uint transactions = 0;

int
database_prepare(char *sql, sqlite3_stmt **stmt)
{
	int result = -1;
	if (sqlite3_prepare(db, sql, strlen(sql) + 1, stmt, NULL) != SQLITE_OK)
	{
		result = 0;
	}
	return result;
}

int
database_execute(char *sql)
{
	int result = -1;
	sqlite3_stmt *stmt;
	if (!database_prepare(sql, &stmt))
	{
		result = 0;
	}
	else
	{
		if (sqlite3_step(stmt) != SQLITE_DONE)
		{
			result = 0;
		}
		sqlite3_finalize(stmt);
	}
	return result;
}

int
database_begin_transaction()
{
	int result = -1;
	if (!transactions)
	{
		if (!database_execute("BEGIN"))
		{
			result = 0;
		}
		else
		{
			transactions++;
		}
	}
	else
	{
		transactions++;
	}
	return result;
}

int
database_rollback_transaction()
{
	int result = -1;
	if (!transactions)
	{
		result = 0;
	}
	else
	{
		if (!database_execute("ROLLBACK"))
		{
			result = 0;
		}
		transactions = 0;
	}
	return result;
}

int
database_commit_transaction()
{
	int result = -1;
	if (!transactions)
	{
		result = 0;
	}
	else if (transactions == 1)
	{
		if (!database_execute("COMMIT"))
		{
			result = 0;
		}
		else
		{
			transactions--;
		}
	}
	else
	{
		transactions--;
	}
	return result;
}

int
database_bind_text(sqlite3_stmt *stmt, const char *key, const char *value)
{
	int result = -1;
	int index = sqlite3_bind_parameter_index(stmt, key);
	if (!index)
	{
		result = 0;
		printf("invalid text parameter index: %s\n", key);
	}
	else
	{
		if (sqlite3_bind_text(stmt, index, value, strlen(value) + 1, SQLITE_STATIC) != SQLITE_OK)
		{
			result = 0;
		}
	}
	return result;
}

int
database_bind_int(sqlite3_stmt *stmt, const char *key, const int value)
{
	int result = -1;
	int index = sqlite3_bind_parameter_index(stmt, key);
	if (!index)
	{
		result = 0;
		printf("invalid int parameter index: %s\n", key);
	}
	else
	{
		if (sqlite3_bind_int(stmt, index, value) != SQLITE_OK)
		{
			result = 0;
		}
	}
	return result;
}

int
database_is_empty(sqlite3 *db)
{
	int result = -1;
	sqlite3_stmt *stmt;
	if (!database_prepare("select count(*) from sqlite_master", &stmt))
	{
		result = 0;
	}
	else
	{
		if (sqlite3_step(stmt) != SQLITE_ROW)
		{
			result = 0;
		}
		else
		{
			if (sqlite3_column_int64(stmt, 0) != 0)
			{
				result = 0;
			}
		}
		sqlite3_finalize(stmt);
	}
	return result;
}

int
database_create_configuration(sqlite3 *db)
{
	int result = -1;
	if (!database_execute("CREATE TABLE configuration (key text not null default '', value text not null default '')"))
	{
		result = 0;
	}
	return result;
}

int
database_create_configuration_index(sqlite3 *db)
{
	int result = -1;
	if (!database_execute("CREATE UNIQUE INDEX configuration_key on configuration (key)"))
	{
		result = 0;
	}
	return result;
}

int
database_prepare_empty(sqlite3 *db)
{
	int result = -1;
	if (database_is_empty(db))
	{
		if (!database_create_configuration(db))
		{
			result = 0;
		}
		else
		{
			if (!database_create_configuration_index(db))
			{
				result = 0;
			}
		}
	}
	return result;
}

int
database_is_valid(sqlite3 *db)
{
	int result = -1;
	sqlite3_stmt *stmt;
	if (!database_prepare("select sqlite_master.sql sql from sqlite_master where type = 'table' and name = 'configuration' and tbl_name = 'configuration'", &stmt))
	{
		result = 0;
	}
	else
	{
		if (sqlite3_step(stmt) != SQLITE_ROW)
		{
			result = 0;
			printf("%s\n", sqlite3_errmsg(db));
		}
		else
		{
			const unsigned char *s = sqlite3_column_text(stmt, 0);
			if (strcmp((char *)s, "CREATE TABLE configuration (key text not null default '', value text not null default '')"))
			{
				result = 0;
			}
		}
		sqlite3_finalize(stmt);
	}
	return result;
}

int
database_upgrade(sqlite3 *db)
{
	int result = -1;
	if (!database_begin_transaction())
	{
		result = 0;
	}
	else
	{
		int database_version;
		if (!configuration_get_int("database_version", &database_version))
		{
			result = 0;
		}
		else
		{
			if (database_version == 0)
			{
				result = database_execute("CREATE TABLE file (hash text not null default '', name text not null default '', path text not null default '')");
				database_version++;
			}
			if (result && database_version == 1)
			{
				result = database_execute("CREATE UNIQUE INDEX file_hash on file (hash)");
				database_version++;
			}
			if (result && database_version == 2)
			{
				result = database_execute("CREATE INDEX file_name on file (name)");
				database_version++;
			}
			if (result && database_version == 3)
			{
				result = database_execute("CREATE INDEX file_path on file (path)");
				database_version++;
			}
			if (result)
			{
				if (!configuration_set_int("database_version", database_version))
				{
					result = 0;
				}
			}
		}
		if (!result)
		{
			database_rollback_transaction();
		}
		else if (!database_commit_transaction())
		{
			result = 0;
		}
	}
	return result;
}

int
database_open()
{
	int result = -1;
	char *filename;
	if ((filename = target_target_base_path_plus_path("backup.sqlite3")) == NULL)
	{
		result = 0;
	}
	else
	{
		if (sqlite3_open(filename, &db) != SQLITE_OK)
		{
			result = 0;
		}
		else
		{
			if (!database_prepare_empty(db))
			{
				result = 0;
			}
			else
			{
				if (!database_is_valid(db))
				{
					result = 0;
				}
				else
				{
					if (!database_upgrade(db))
					{
						result = 0;
					}
				}
			}
		}
		free(filename);
	}
	return result;
}

int
database_close()
{
	int result = -1;
	if (sqlite3_close(db) != SQLITE_OK)
	{
		result = 0;
	}
	return result;
}
