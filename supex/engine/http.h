#pragma once

#include "http_parser.h"

#undef TRUE
#define TRUE 1
#undef FALSE
#define FALSE 0

struct http_status {
	enum http_method method;
	int num_headers;
	int status_code;

	int url_offset;
	int url_len;
	int path_offset;
	int uri_offset;
	unsigned short path_len;
	unsigned short uri_len;
	int body_offset;
	size_t body_size;

	enum { NONE=0, FIELD, VALUE } last_header_element;
	enum http_errno err;
#if 0
#define MAX_HEADERS 13
#define MAX_ELEMENT_SIZE 2048
	char headers [MAX_HEADERS][2][MAX_ELEMENT_SIZE];
#endif	
	unsigned short should_keep_alive;
	unsigned short http_major;
	unsigned short http_minor;
	unsigned short step;
	unsigned short over;
	
	unsigned int dosize;
};

struct http_parse_info {
	http_parser hp;
	struct http_status hs;
};
