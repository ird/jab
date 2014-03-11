/*
 * jab_common.h
 *
 *  Created on: 4/03/2014
 *      Author: Stacey Richards
 *
 * The author disclaims copyright to this source code. In place of a legal notice, here is a blessing:
 *
 *    May you do good and not evil.
 *    May you find forgiveness for yourself and forgive others.
 *    May you share freely, never taking more than you give.
 */

#ifndef JAB_COMMON_H_
#define JAB_COMMON_H_

/*
 * From time to time we'll be needing to append a filename (usually from a struct dirent->d_name) to a path but sometimes not.
 * We'll have a variety of possible input combinations:
 *
 * path filename result
 * ---- -------- -------------------
 * -    *        error, returns NULL
 * /    -        /
 * /a   -        /a
 * /a/  -        /a
 * /    b        /b
 * /a   b        /a/b
 * /a/  b        /a/b
 * a    b        a/b
 * .    b        ./b
 * ./   b        ./b
 *
 * where - means empty string and * means any string.
 *
 * Any empty path is invalid and will return NULL which indicates an error. In all other situations we must ensure that the resulting
 * path (input path plus input filename) separates the input path and input filename with one and only only one slash unless the filename
 * starts with a slash (which is never expected).
 *
 * Therefore, path should never be empty and may or may not end in a slash and filename may be empty and should not start with a slash.
 *
 * Returns a string with the filename appended to the path which needs to be freed by the caller.
 */
char *
append_filename_to_path(const char *path, const char *filename);

/*
 * Returns -1 if the path exists otherwise returns 0.
 */
int
directory_exists(const char *path);

/*
 * Returns -1 if the path exists. If the path does not exist, attempts to create the path and returns -1 if successful otherwise returns 0.
 */
int
create_directory_if_not_exists(const char *path, mode_t mode);

/*
 * Given the p_path and p_sha1_string returns a path representation of the p_sha1_string and tries to ensure that the returned path can be
 * used to create a file. The input path must exist (this function will fail if it doesn't) prior to the call. The caller must free the returned
 * path.
 */
char *
sha1_string_to_path(const char *p_path, const char *p_sha1_string);

#endif /* JAB_COMMON_H_ */
