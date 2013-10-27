/*
 * configuration.c
 *
 *  Created on: 30/09/2013
 *      Author: stacey
 *
 * The author disclaims copyright to this source code. In place of a legal notice, here is a blessing:
 *
 *    May you do good and not evil.
 *    May you find forgiveness for yourself and forgive others.
 *    May you share freely, never taking more than you give.
 */

#include <sqlite3.h>

#include "database.h"

int
configuration_get_int(const char *p_key, int *p_value)
{
	int result = -1;
	sqlite3_stmt *stmt;
	if (!database_prepare("select configuration.value value from configuration where configuration.key = :key", &stmt))
	{
		result = 0;
	}
	else
	{
		if (!database_bind_text(stmt, ":key", p_key))
		{
			result = 0;
		}
		else
		{
			int rc = sqlite3_step(stmt);
			if (rc == SQLITE_DONE)
			{
				*p_value = 0;
			}
			else if (rc == SQLITE_ROW)
			{
				*p_value = sqlite3_column_int(stmt, 0);
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
configuration_update_int(const char *p_key, int p_value)
{
	int result = -1;
	sqlite3_stmt *stmt;
	if (!database_prepare("update configuration set value = :value where key = :key", &stmt))
	{
		result = 0;
	}
	else
	{
		if (!database_bind_int(stmt, ":value", p_value))
		{
			result = 0;
		}
		else if (!database_bind_text(stmt, ":key", p_key))
		{
			result = 0;
		}
		else if (sqlite3_step(stmt) != SQLITE_DONE)
		{
			result = 0;
		}
		sqlite3_finalize(stmt);
	}
	return result;
}

int
configuration_insert_int(const char *p_key, int p_value)
{
	int result = -1;
	sqlite3_stmt *stmt;
	if (!database_prepare("insert into configuration (key, value) values (:key, :value)", &stmt))
	{
		result = 0;
	}
	else
	{
		if (!database_bind_text(stmt, ":key", p_key))
		{
			result = 0;
		}
		else if (!database_bind_int(stmt, ":value", p_value))
		{
			result = 0;
		}
		else if (sqlite3_step(stmt) != SQLITE_DONE)
		{
			result = 0;
		}
		sqlite3_finalize(stmt);
	}
	return result;
}

int
configuration_set_int(const char *p_key, int p_value)
{
	int result = -1;
	sqlite3_stmt *stmt;
	if (!p_value)
	{
		if (database_prepare("delete from configuration where key = :key", &stmt))
		{
			result = 0;
		}
		else
		{
			if (!database_bind_text(stmt, ":key", p_key))
			{
				result = 0;
			}
			else if (sqlite3_step(stmt) != SQLITE_DONE)
			{
				result = 0;
			}
			sqlite3_finalize(stmt);
		}
	}
	else
	{
		if (!database_begin_transaction())
		{
			result = 0;
		}
		else
		{
			if (!database_prepare("select configuration.value value from configuration where configuration.key = :key", &stmt))
			{
				result = 0;
			}
			else
			{
				if (!database_bind_text(stmt, ":key", p_key))
				{
					result = 0;
				}
				else
				{
					int rc = sqlite3_step(stmt);
					if (rc == SQLITE_DONE)
					{
						if (!configuration_insert_int(p_key, p_value))
						{
							result = 0;
						}
					}
					else if (rc == SQLITE_ROW)
					{
						if (sqlite3_column_int(stmt, 0) != p_value)
						{
							if (!configuration_update_int(p_key, p_value))
							{
								result = 0;
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
			if (!result)
			{
				database_rollback_transaction();
			}
			else if (!database_commit_transaction())
			{
				result = 0;
			}
		}
	}
	return result;
}

