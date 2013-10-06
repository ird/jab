/*
 * hash.h
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

#ifndef HASH_H_
#define HASH_H_

#include <openssl/sha.h>

extern const size_t SHA1_STRING_SIZE;

/* p_path is the absolute path and filename of the file that we're going to calculate the sha1 for.
 * p_hash_string is a 41 byte block of space that we'll use to populate with the string representation of the file's sha1.
 * the caller is responsible for allocating the 41 byte p_hash_string block and responsible for freeing it.
 * the first 40 bytes of p_hash_string will be hex characters and the final byte will be a 0 (if the function returns 0).
 * on success the function returns -1.
 * on error the function returns 0.
 */
int
hash_file_to_string(const char *p_path, char *p_hash_string);

#endif /* FILE_TO_SHA1_STRING_H_ */
