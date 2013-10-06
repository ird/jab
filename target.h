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

char *
target_target_base_path_plus_path(char *p_path);

int
target_setup(char *p_path);

int
target_cleanup();

int
target_receive_file();

int
target_receive_path();

int
target_receive_hash();

int
target_receive_data();

#endif /* TARGET_H_ */
