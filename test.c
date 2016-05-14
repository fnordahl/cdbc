/*
 *  test.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cdbc.h"

int main(int argc, char **argv) {
	CDBC cdbc;
	CDBC_QUERY q;
	CDBC_RESULT res;
	int count = 0;
	char *name;
	
	cdbc = cdbc_init();
	
	if (cdbc_connect(cdbc, "FreeTDS", "TDS_VERSION=7.0", "192.168.0.10", 1433,
				 "username", "password"))
	{
		printf("connection failed: %s\n", cdbc_error(cdbc));
		return 0;
	}

	q = NULL;
	q = cdbc_prepare(cdbc, "SELECT 'test' as test");
	
	printf("Resultset has %d columns\n", cdbc_c_count(q));
	if (cdbc_execute(q)) {
		printf("query failed: %s\n", cdbc_query_error(q));
		return 0;
	}

	name = cdbc_c_name(q, "test");
	res = cdbc_fetch(q);
	while (res) {
		printf("%s\n", name);
		count++;
		res = cdbc_fetch(q);
	}
	printf("fetched %d rows.\n", count);
	cdbc_finish(q);
	
	if (cdbc_disconnect(cdbc))
		printf("disconnect failed\n");
	
	cdbc_cleanup(cdbc);
	
	return 0;
}
