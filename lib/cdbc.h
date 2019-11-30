/*
 *  cdbc.h
 *  cdbc
 *
 *  Created by Frode Nordahl on 14.11.08.
 *  Copyright 2008 Frode Nordahl. All rights reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#ifndef _CDBC_H
#define _CDBC_H

#include <sys/types.h>

extern signed short CDBC_C_CHAR;
extern signed short CDBC_C_WCHAR;
extern signed short CDBC_C_SSHORT;
extern signed short CDBC_C_USHORT;
extern signed short CDBC_C_SLONG;
extern signed short CDBC_C_ULONG;
extern signed short CDBC_C_FLOAT;
extern signed short CDBC_C_DOUBLE;
extern signed short CDBC_C_BIT;
extern signed short CDBC_C_STINYINT;
extern signed short CDBC_C_UTINYINT;
extern signed short CDBC_C_SBIGINT;
extern signed short CDBC_C_UBIGINT;
extern signed short CDBC_C_BINARY;
extern signed short CDBC_C_BOOKMARK;
extern signed short CDBC_C_VARBOOKMARK;
extern signed short CDBC_C_TYPE_DATE;
extern signed short CDBC_C_TYPE_TIME;
extern signed short CDBC_C_TYPE_TIMESTAMP;
extern signed short CDBC_C_NUMERIC;
extern signed short CDBC_C_GUID;
extern signed short CDBC_C_INTERVAL_YEAR;
extern signed short CDBC_C_INTERVAL_MONTH;
extern signed short CDBC_C_INTERVAL_DAY;
extern signed short CDBC_C_INTERVAL_HOUR;
extern signed short CDBC_C_INTERVAL_MINUTE;
extern signed short CDBC_C_INTERVAL_SECOND;
extern signed short CDBC_C_INTERVAL_YEAR_TO_MONTH;
extern signed short CDBC_C_INTERVAL_DAY_TO_HOUR;
extern signed short CDBC_C_INTERVAL_DAY_TO_MINUTE;
extern signed short CDBC_C_INTERVAL_DAY_TO_SECOND;
extern signed short CDBC_C_INTERVAL_HOUR_TO_MINUTE;
extern signed short CDBC_C_INTERVAL_HOUR_TO_SECOND;
extern signed short CDBC_C_INTERVAL_MINUTE_TO_SECOND;

typedef struct _CDBC* CDBC;
typedef struct _CDBC_QUERY* CDBC_QUERY;

struct _CDBC_RESULT {
	char **row_name;
	char **row_data;
	size_t **row_len;
	long **row_ind;
};

typedef struct _CDBC_RESULT* CDBC_RESULT;


CDBC cdbc_init(void);
void cdbc_cleanup(CDBC);

int cdbc_connect(CDBC, char *driver, char *driver_opt, char *server, int port, char *uid, char *pwd);
int cdbc_disconnect(CDBC);
char *cdbc_error(CDBC);

CDBC_QUERY cdbc_prepare(CDBC, char *);
int cdbc_bind_param(CDBC_QUERY, int, signed short, void*, size_t);
int cdbc_execute(CDBC_QUERY);
CDBC_RESULT cdbc_fetch(CDBC_QUERY q);
void cdbc_finish(CDBC_QUERY);
char *cdbc_query_error(CDBC_QUERY);

short cdbc_c_count(CDBC_QUERY q);
char *cdbc_c_name(CDBC_QUERY, char*);
char *cdbc_c_id(CDBC_QUERY, int);

#endif
