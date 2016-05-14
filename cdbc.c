/*
 *  cdbc.c
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

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sql.h>
#include <sqlext.h>

#include "cdbc.h"

struct _CDBC {
			/* ODBC Environment Handle */
	SQLHENV	henv;
			/* ODBC Connection Handle */
	SQLHDBC	hdbc;
	
	char errstr[SQL_MAX_MESSAGE_LENGTH*2];
};

struct _CDBC_QUERY {
	/* CDBC Handle we are referencing */
	CDBC		cdbc;
	
	/* ODBC Statement Handle */
	SQLHSTMT	hstmt;
	SQLCHAR		query[1024];
	SQLINTEGER	query_len;
	int		is_error;
	
	/* Resultset private data */
	char		*row_buf;
	SQLLEN		**param_ind;
	short		col_count;
	
	/* Resultset public data */
	struct _CDBC_RESULT *res;
	
	char errstr[SQL_MAX_MESSAGE_LENGTH*2];
};

SQLSMALLINT CDBC_C_CHAR	= SQL_C_CHAR;
SQLSMALLINT CDBC_C_WCHAR = SQL_C_WCHAR;
SQLSMALLINT CDBC_C_SSHORT = SQL_C_SSHORT;
SQLSMALLINT CDBC_C_USHORT = SQL_C_USHORT;
SQLSMALLINT CDBC_C_SLONG = SQL_C_SLONG;
SQLSMALLINT CDBC_C_ULONG = SQL_C_ULONG;
SQLSMALLINT CDBC_C_FLOAT = SQL_C_FLOAT;
SQLSMALLINT CDBC_C_DOUBLE = SQL_C_FLOAT;
SQLSMALLINT CDBC_C_BIT = SQL_C_BIT;
SQLSMALLINT CDBC_C_STINYINT = SQL_C_STINYINT;
SQLSMALLINT CDBC_C_UTINYINT = SQL_C_UTINYINT;
SQLSMALLINT CDBC_C_SBIGINT = SQL_C_SBIGINT;
SQLSMALLINT CDBC_C_UBIGINT = SQL_C_UBIGINT;
SQLSMALLINT CDBC_C_BINARY = SQL_C_BINARY;
SQLSMALLINT CDBC_C_BOOKMARK	= SQL_C_BOOKMARK;
SQLSMALLINT CDBC_C_VARBOOKMARK = SQL_C_VARBOOKMARK;
SQLSMALLINT CDBC_C_TYPE_DATE = SQL_C_TYPE_DATE;
SQLSMALLINT CDBC_C_TYPE_TIME = SQL_C_TYPE_TIME;
SQLSMALLINT CDBC_C_TYPE_TIMESTAMP = SQL_C_TYPE_TIMESTAMP;
SQLSMALLINT CDBC_C_NUMERIC = SQL_C_NUMERIC;
SQLSMALLINT CDBC_C_GUID = SQL_C_GUID;
SQLSMALLINT CDBC_C_INTERVAL_YEAR = SQL_C_INTERVAL_YEAR;
SQLSMALLINT CDBC_C_INTERVAL_MONTH = SQL_C_INTERVAL_MONTH;
SQLSMALLINT CDBC_C_INTERVAL_DAY = SQL_C_INTERVAL_DAY;
SQLSMALLINT CDBC_C_INTERVAL_HOUR = SQL_C_INTERVAL_HOUR;
SQLSMALLINT CDBC_C_INTERVAL_MINUTE = SQL_C_INTERVAL_MINUTE;
SQLSMALLINT CDBC_C_INTERVAL_SECOND = SQL_C_INTERVAL_SECOND;
SQLSMALLINT CDBC_C_INTERVAL_YEAR_TO_MONTH = SQL_C_INTERVAL_YEAR_TO_MONTH;
SQLSMALLINT CDBC_C_INTERVAL_DAY_TO_HOUR = SQL_C_INTERVAL_DAY_TO_HOUR;
SQLSMALLINT CDBC_C_INTERVAL_DAY_TO_MINUTE = SQL_C_INTERVAL_DAY_TO_MINUTE;
SQLSMALLINT CDBC_C_INTERVAL_DAY_TO_SECOND = SQL_C_INTERVAL_DAY_TO_SECOND;
SQLSMALLINT CDBC_C_INTERVAL_HOUR_TO_MINUTE = SQL_C_INTERVAL_HOUR_TO_MINUTE;
SQLSMALLINT CDBC_C_INTERVAL_HOUR_TO_SECOND = SQL_C_INTERVAL_HOUR_TO_SECOND;
SQLSMALLINT CDBC_C_INTERVAL_MINUTE_TO_SECOND = SQL_C_INTERVAL_MINUTE_TO_SECOND;

char *_cdbc_get_error(char *errstr, SQLSMALLINT h_type, SQLHANDLE handle) {
	int errstr_len = 0;
	SQLCHAR sqlstate[6], msg[SQL_MAX_MESSAGE_LENGTH];
	SQLINTEGER native_err;
	SQLSMALLINT i, msg_len;
	SQLRETURN rc;
	
	*errstr = '\0';
	
	i = 1;
	while (errstr_len < ((SQL_MAX_MESSAGE_LENGTH*2)-1) &&
		(rc = SQLGetDiagRec(h_type, handle, i, sqlstate, &native_err, msg, sizeof(msg), &msg_len)) != SQL_NO_DATA)
	{
		errstr_len += snprintf(errstr+errstr_len, (SQL_MAX_MESSAGE_LENGTH*2)-errstr_len,
							  "sqlstate='%.*s' native=%d msg='%.*s'", sizeof(sqlstate), sqlstate,
							   native_err, msg_len, msg);
		i++;
	}
	
	return errstr;
}

char *cdbc_error(CDBC cdbc) {
	return _cdbc_get_error(cdbc->errstr, SQL_HANDLE_DBC, cdbc->hdbc);
}

char *cdbc_query_error(CDBC_QUERY q) {
	return _cdbc_get_error(q->errstr, SQL_HANDLE_STMT, q->hstmt);
}

CDBC cdbc_init() {
	CDBC cdbc;
	SQLRETURN rc;
	
	if ((cdbc = (CDBC) malloc(sizeof(*cdbc))) == NULL)
		return NULL;

	cdbc->henv = SQL_NULL_HENV;
	cdbc->hdbc = SQL_NULL_HDBC;
	
	/* Allocate ODBC Environment */
	rc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &cdbc->henv);
	if (!SQL_SUCCEEDED(rc))
		goto error;
	
	/* Set ODBC version */
	int64_t value = SQL_OV_ODBC3;
	rc = SQLSetEnvAttr(cdbc->henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)value, 0);
	if (!SQL_SUCCEEDED(rc))
		goto error;
	
	/* Allocate ODBC Connection */
	rc = SQLAllocHandle(SQL_HANDLE_DBC, cdbc->henv, &cdbc->hdbc);
	if (!SQL_SUCCEEDED(rc))
		goto error;
	
	return cdbc;
	
error:
	if (cdbc != NULL)
		cdbc_cleanup(cdbc);
	
	return NULL;
}

void cdbc_cleanup(CDBC cdbc) {
	if (cdbc->hdbc != SQL_NULL_HDBC) {
		SQLDisconnect(cdbc->hdbc);
		SQLFreeHandle(SQL_HANDLE_DBC, cdbc->hdbc);
	}
	
	if (cdbc->henv != SQL_NULL_HENV)
		SQLFreeHandle(SQL_HANDLE_ENV, cdbc->henv);
	
	free (cdbc);
}

int cdbc_connect(CDBC cdbc, char *driver, char *driver_opt, char *server, int port, char *uid, char *pwd) {
	SQLRETURN rc;
	SQLCHAR dsn_in[1024];
	SQLCHAR dsn_out[1024];
	SQLSMALLINT dsn_len;

	*dsn_in = '\0';
	snprintf((char*)dsn_in, sizeof(dsn_in), "DRIVER=%s;%s;SERVER=%s;PORT=%d;UID=%s;PWD=%s",
		 driver, driver_opt, server, port, uid, pwd);
	rc = SQLDriverConnect(cdbc->hdbc, NULL, dsn_in, strlen((char*)dsn_in),
			      dsn_out, sizeof(dsn_out), &dsn_len, SQL_DRIVER_NOPROMPT);
	
	if (!SQL_SUCCEEDED(rc)) {
		return -1;
	}
	
	return 0;
}

int cdbc_disconnect(CDBC cdbc) {
	SQLRETURN rc;
	
	rc = SQLDisconnect(cdbc->hdbc);
	
	if (!SQL_SUCCEEDED(rc)) {
		return -1;
	}
	
	return 0;
}

int _cdbc_res_init(CDBC_QUERY q) {
	SQLRETURN rc;
	SQLSMALLINT i;
	SQLCHAR col_name[256];
	SQLSMALLINT col_name_len, col_type, col_scale, col_nullable;
	SQLULEN col_size, disp_size, sum_disp_size;
	char *cp;
	
	/* Allocate memory for result structure */
	q->res = malloc(sizeof(*q->res));
		
	/* Allocate memory for row arrays */
	q->res->row_name = malloc(sizeof(char*)*(q->col_count+1));
	q->res->row_data = malloc(sizeof(char*)*(q->col_count+1));
	q->res->row_len = malloc(sizeof(size_t*)*(q->col_count+1));
	for (i = 0; i < q->col_count; i++) {
		q->res->row_len[i] = malloc(sizeof(size_t));
	}
	q->res->row_ind = malloc(sizeof(SQLLEN*)*(q->col_count+1));
	for (i = 0; i < q->col_count; i++) {
		q->res->row_ind[i] = malloc(sizeof(SQLLEN));
	}
	
	/* Terminate them */
	q->res->row_name[q->col_count] = NULL;
	q->res->row_data[q->col_count] = NULL;
	q->res->row_len[q->col_count] = NULL;
	q->res->row_ind[q->col_count] = NULL;
	
	/*
	 * Call SQLDescribeCol for each column in the result set
	 *
	 * 1. Store name of column
	 * 2. Determine max string length for each column, allocate memory for rowbuffer
	 */
	sum_disp_size = 0;
	for (i = 0; i < q->col_count; i++) {
		rc = SQLDescribeCol(q->hstmt, i+1, col_name, sizeof(col_name), &col_name_len, &col_type,
				    &col_size, &col_scale, &col_nullable);
		if (!SQL_SUCCEEDED(rc)) {
			q->is_error = 1;
			return -1;
		}
		/*
		 * Sizes based on Display Size information from the ODBC specification:
		 * http://msdn.microsoft.com/en-us/library/ms713974(VS.85).aspx
		 */
		switch(col_type) {
			case SQL_CHAR:
			case SQL_VARCHAR:
			case SQL_LONGVARCHAR:
				disp_size = col_size;
				break;
			case SQL_WCHAR:
			case SQL_WVARCHAR:
			case SQL_WLONGVARCHAR:
				disp_size = (col_size*2);
				break;
			case SQL_DECIMAL:
			case SQL_NUMERIC:
				disp_size = (col_scale+2);
				break;
			case SQL_BIT:
				disp_size = 1;
				break;
			case SQL_TINYINT:
				disp_size = 4;
				break;
			case SQL_SMALLINT:
				disp_size = 6;
				break;
			case SQL_INTEGER:
				disp_size = 11;
				break;
			case SQL_BIGINT:
				disp_size = 20;
				break;
			case SQL_REAL:
				disp_size = 14;
				break;
			case SQL_FLOAT:
			case SQL_DOUBLE:
				disp_size = 24;
				break;
			case SQL_BINARY:
			case SQL_VARBINARY:
			case SQL_LONGVARBINARY:
				disp_size = (col_size*2);
				break;
			case SQL_TYPE_DATE:
				disp_size = 10;
				break;
			case SQL_TYPE_TIME:
				disp_size = 9;
				break;
			case SQL_TYPE_TIMESTAMP:
				disp_size = 20;
				break;
			case SQL_GUID:
				disp_size = 36;
			default:
				disp_size = 255;
				break;
		}
		q->res->row_name[i] = strdup((char*)col_name);
		*q->res->row_len[i] = disp_size;
		sum_disp_size += disp_size;
	}
	/* Allocate memory pool for data */
	q->row_buf = malloc(sum_disp_size);
	
	/* 
	 * Point row_data pointers to its position in the memory pool.
	 * Bind them to row data.
	 */
	cp = q->row_buf;
	for (i = 0; i < q->col_count; i++) {
		q->res->row_data[i] = cp;
		*q->res->row_ind[i] = 0;
		rc = SQLBindCol(q->hstmt, i+1, SQL_C_CHAR, q->res->row_data[i], *q->res->row_len[i], q->res->row_ind[i]);
		cp += *q->res->row_len[i];
	}
	
	return 0;
}

void _cdbc_res_cleanup(CDBC_QUERY q) {
	int i;
	
	free (q->row_buf);
	for (i = 0; i <= q->col_count; i++) {
		free(q->res->row_ind[i]);
		free(q->res->row_len[i]);
		free(q->res->row_name[i]);
	}
	
	free (q->res->row_ind);
	free (q->res->row_len);
	free (q->res->row_data);
	free (q->res->row_name);
	free (q->res);
	
	q->res = NULL;
}

CDBC_QUERY cdbc_prepare(CDBC cdbc, char* query) {
	CDBC_QUERY q;
	SQLRETURN rc;
	int i;
	
	if ((q = malloc(sizeof(*q))) == NULL)
		return NULL;
	
	q->cdbc = cdbc;
	q->hstmt = SQL_NULL_HSTMT;
	
	*q->query = '\0';
	strncat((char*)q->query, query, sizeof(q->query));
	q->query_len = strlen((char*)q->query);
	
	q->is_error = 0;
	q->res = NULL;
	
	rc = SQLAllocHandle(SQL_HANDLE_STMT, q->cdbc->hdbc, &q->hstmt);
	if (!SQL_SUCCEEDED(rc)) {
		free (q);
		return NULL;
	}
	
	rc = SQLPrepare(q->hstmt, q->query, q->query_len);
	if (!SQL_SUCCEEDED(rc)) {
		/*
		 * Our caller needs the resources allocated by this function to determine what
		 * went wrong.
		 *
		 * Flag that an error condition exists and return CDBC_QUERY object.
		 * Caller will be notified of the error at the next call to cdbc_execute
		 */
		q->is_error = 1;
		return q;
	}
	
	/* Get number of columns in result set */
	rc = SQLNumResultCols(q->hstmt, &q->col_count);
	if (!SQL_SUCCEEDED(rc)) {
		q->is_error = 1;
		return q;
	}	
	
	_cdbc_res_init(q);
	
	q->param_ind = malloc(sizeof(SQLLEN*)*(q->col_count+1));
	for (i = 0; i < q->col_count; i++) {
		q->param_ind[i] = malloc(sizeof(SQLLEN));
	}
	q->param_ind[q->col_count] = NULL;
	
	return q;
}

int cdbc_bind_param(CDBC_QUERY q, int p_num, signed short p_type, void *data, size_t data_len) {
	SQLRETURN rc;
	SQLSMALLINT par_sqltype, par_scale, par_nullable;
	SQLULEN par_size;

	/* set indicator pointer to proper value */
	if (data == NULL) {
		*q->param_ind[p_num] = SQL_NULL_DATA;
	} else {
		switch (p_type) {
			case SQL_C_CHAR:
			case SQL_C_WCHAR:
				*q->param_ind[p_num] = data_len;
				break;
			default:
				*q->param_ind[p_num] = 0;
				break;
		}
	}
	
	/*
	 * check that data has the alignment required by p_type
	 *
	 * ref: http://www.unixodbc.org/doc/ODBC64.html
	 *
	 */
	switch (p_type) {
		case SQL_C_WCHAR:
			/* must be a multiple of 2 */
			assert (data_len % 2 == 0);
			break;
		case SQL_C_SSHORT:
		case SQL_C_USHORT:
		case SQL_C_SHORT:
		case SQL_C_DATE:
		case SQL_C_TIMESTAMP:
		case SQL_C_TYPE_DATE:
		case SQL_C_TYPE_TIME:
		case SQL_C_TYPE_TIMESTAMP:
			assert (data_len == 2);
			break;
		case SQL_C_SLONG:
		case SQL_C_ULONG:
		case SQL_C_LONG:
		case SQL_C_FLOAT:
			assert (data_len == 4);
			break;
		case SQL_C_DOUBLE:
		case SQL_C_SBIGINT:
		case SQL_C_UBIGINT:
			assert (data_len == 8);
			break;
	}
	
	rc = SQLDescribeParam(q->hstmt, p_num, &par_sqltype, &par_size, &par_scale, &par_nullable);
	if (SQL_SUCCEEDED(rc)) {
		rc = SQLBindParameter(q->hstmt, p_num, SQL_PARAM_INPUT, p_type, par_sqltype, par_size, par_scale, data, data_len, q->param_ind[p_num]);
	} else {
		/*
		 * assume that SQLDescribeParam is not implemented in the driver,
		 * specify SQL_VARCHAR and hope the server converts the data
		 * according to actual target data type.
		 */
		rc = SQLBindParameter(q->hstmt, p_num, SQL_PARAM_INPUT, p_type, SQL_VARCHAR, 255, 0, data, data_len, q->param_ind[p_num]);
	}
	
	return 0;
}

int cdbc_execute(CDBC_QUERY q) {
	SQLRETURN rc;
	
	if (q->is_error) {
		/* A previous error, probably from cdbc_prepare, prohibits us from continuing. */
		return -1;
	}
	
	/* Close any open cursors */
	SQLCloseCursor(q->hstmt);
	
	rc = SQLExecute(q->hstmt);
	if (!SQL_SUCCEEDED(rc)) {
		q->is_error = 1;
		return -1;
	}
	
	return 0;
}

CDBC_RESULT cdbc_fetch(CDBC_QUERY q) {
	SQLRETURN rc;
	int i;
	
	rc = SQLFetch(q->hstmt);
	if (!SQL_SUCCEEDED(rc)) {
		if (rc == SQL_NO_DATA)
			SQLCloseCursor(q->hstmt);
		else
			q->is_error = 1;
		return NULL;
	}
	
	/* relieve caller of the duty of checking indicator for NULL values */
	for (i = 0; i < q->col_count; i++) {
		if (*q->res->row_ind[i] == SQL_NULL_DATA)
			*q->res->row_data[i] = '\0';
	}
	return q->res;
}

void cdbc_finish(CDBC_QUERY q) {
	int i;
	
	if (q->res != NULL) {
		for (i = 0; i <= q->col_count; i++)
			free(q->param_ind[i]);
		
		free (q->param_ind);
		
		_cdbc_res_cleanup(q);
	}
	
	SQLCancel(q->hstmt);
	SQLFreeHandle(SQL_HANDLE_STMT, q->hstmt);
	free(q);
}

short cdbc_c_count(CDBC_QUERY q) {
	return q->col_count;
}

/* Get pointer to column by name */
char *cdbc_c_name(CDBC_QUERY q, char *col_name) {
	int i;
	
	for (i = 0; i < q->col_count; i++) {
		if (strcasecmp(q->res->row_name[i], col_name) == 0) {
			return q->res->row_data[i];
		}
	}
	return NULL;
}

/* Get pointer to column by number */
char *cdbc_c_id(CDBC_QUERY q, int i) {
	if (i >= 0 && i < q->col_count)
		return q->res->row_data[i];
	else
		return NULL;
}
