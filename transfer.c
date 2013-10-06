/*
 * transfer.c
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

#include "transfer.h"

#include "target.h"

void *m_transfer_buffer[TRANSFER_BUFFER_SIZE];

int
transfer_file_to_target()
{
	return target_receive_file();
}

int
transfer_path_to_target()
{
	return target_receive_path();
}

int
transfer_hash_to_target()
{
	return target_receive_hash();
}

int
transfer_data_to_target()
{
	return target_receive_data();
}
