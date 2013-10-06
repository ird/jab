/*
 * source.h
 *
 *  Created on: 27/09/2013
 *      Author: Stacey Richards
 *
 * The author disclaims copyright to this source code. In place of a legal notice, here is a blessing:
 *
 *    May you do good and not evil.
 *    May you find forgiveness for yourself and forgive others.
 *    May you share freely, never taking more than you give.
 */

#ifndef SOURCE_H_
#define SOURCE_H_

extern unsigned long m_source_directories_found;

extern unsigned long m_source_directories_skipped;

extern unsigned long m_source_files_found;

extern unsigned long m_source_files_skipped;

int
source(char *p_path);

#endif /* SOURCE_H_ */
