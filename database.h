/*
 * database.h
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

#ifndef DATABASE_H_
#define DATABASE_H_

#include <sqlite3.h>

#include "target.h"

int
database_open(char *p_path);

int
database_close();

int
database_execute(char *sql);

int
database_prepare(char *sql, sqlite3_stmt **stmt);

int
database_bind_text(sqlite3_stmt *stmt, const char *key, const char *value);

int
database_bind_int(sqlite3_stmt *stmt, const char *key, const int value);

int
database_begin_transaction();

int
database_rollback_transaction();

int
database_commit_transaction();

#endif /* DATABASE_H_ */
