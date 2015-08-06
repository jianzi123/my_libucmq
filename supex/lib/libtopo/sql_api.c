#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "sql_api.h"

int64_t mysql_cmd( struct sql_info *info, SQLFUN_CALLBACK excb, void *args )
{
	MYSQL conn;
	MYSQL_RES *res_ptr;
	MYSQL_ROW res_row;

	int64_t i = 0, row = 0;

	assert( info && excb );

	mysql_init(&conn);
	if(!mysql_real_connect(&conn, (info->host)?info->host:"localhost",
				info->username, info->password, info->database, info->port, NULL, 0)) {
		fprintf(stderr, "Connection failed\n"); 
		if (mysql_errno(&conn)) {
			printf("Connection error %d: %s\n", mysql_errno(&conn), mysql_error(&conn));
		}
		return ERR_MYSQL_CONNECT_FAILED;
	}
	if(mysql_query(&conn, info->sql)) {
		printf("SELECT error:%s\n",mysql_error(&conn)); 
		mysql_close(&conn);
		return ERR_MYSQL_COMMAND_FAILED;
	}
	res_ptr = mysql_store_result(&conn);
	if(!res_ptr) {
		printf("mysql error:%s\n",mysql_error(&conn)); 
		mysql_free_result(res_ptr);
		mysql_close(&conn);
		return ERR_MYSQL_DISPOSE_FAILED;
	}
	row = mysql_num_rows(res_ptr);
	printf("Retrieved %ld Rows\n", row); 
	while( (res_row = mysql_fetch_row(res_ptr)) ) {
	       i++;
	       unsigned long *lengths = mysql_fetch_lengths( res_ptr );
	       excb( res_row, lengths, args );
	} 
	if ( (i != row) && mysql_errno(&conn) ) { 
		printf("Retrive error:%s\n", mysql_error(&conn)); 
	} 
	mysql_free_result(res_ptr);
	mysql_close(&conn);
	return row;
}
