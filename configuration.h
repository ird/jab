/*
 * configuration.h
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

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

int
configuration_get_int(const char *p_key, int *p_value);

int
configuration_set_int(const char *p_key, int p_value);

int
configuration_get_text(const char *p_key, char **p_value);

int
configuration_set_text(const char *p_key, char *p_value);

#endif /* CONFIGURATION_H_ */
