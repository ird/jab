/*
 * transfer.h
 *
 *  Created on: 30/09/2013
 *      Author: Stacey Richards
 *
 * The author disclaims copyright to this source code. In place of a legal notice, here is a blessing:
 *
 *    May you do good and not evil.
 *    May you find forgiveness for yourself and forgive others.
 *    May you share freely, never taking more than you give.
 */

#ifndef TRANSFER_H_
#define TRANSFER_H_

#define TRANSFER_BUFFER_SIZE 4096

extern void *m_transfer_buffer[TRANSFER_BUFFER_SIZE];

int
transfer_file_to_target();

int
transfer_done_to_target();

int
transfer_path_to_target();

int
transfer_back_to_target();

int
transfer_hash_to_target();

int
transfer_data_to_target();

int
transfer_stop_to_target();


#endif /* TRANSFER_H_ */
