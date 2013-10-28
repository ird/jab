/*
 * target.h
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

#ifndef TARGET_H_
#define TARGET_H_

int
target_init(char *p_path, char *p_name);

void
target_free();

int
target_receive_file();

int
target_receive_info();

int
target_receive_hash();

int
target_receive_data();

int
target_receive_done();

int
target_receive_path();

int
target_receive_back();

int
target_receive_stop();

#endif /* TARGET_H_ */
