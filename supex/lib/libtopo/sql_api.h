#pragma once

#include <mysql.h>
#include <stdint.h>

#define ERR_MYSQL_CONNECT_FAILED	-1
#define ERR_MYSQL_COMMAND_FAILED	-2
#define ERR_MYSQL_DISPOSE_FAILED	-3

struct sql_info {
	char *host;
	unsigned int port;
	char *username;
	char *password;
	char *database;
	char *sql;
};

typedef void (*SQLFUN_CALLBACK)( MYSQL_ROW data, unsigned long *lens, void *args );

int64_t mysql_cmd( struct sql_info *info, SQLFUN_CALLBACK excb, void *args );
