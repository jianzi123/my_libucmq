#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "http_parser.h"
#include <ctype.h>

#define MAX_HEADERS 13
#define MAX_ELEMENT_SIZE 2048

#define BUFF_LEN 4096
size_t strlncat(char *dst, size_t len, const char *src, size_t n)
{
	size_t slen;
	size_t dlen;
	size_t rlen;
	size_t ncpy;

	slen = strnlen(src, n);
	dlen = strnlen(dst, len);

	if (dlen < len) {
		rlen = len - dlen;
		ncpy = slen < rlen ? slen : (rlen - 1);
		memcpy(dst + dlen, src, ncpy);
		dst[dlen + ncpy] = '\0';
	}

	assert(len > slen + dlen);
	return slen + dlen;
}
/*********************************************************/
struct message {
	//const char *raw;
	//enum http_parser_type type;
	enum http_method method;
	int status_code;
	char request_path[MAX_ELEMENT_SIZE];
	char request_url[MAX_ELEMENT_SIZE];
	//char fragment[MAX_ELEMENT_SIZE];
	//char query_string[MAX_ELEMENT_SIZE];
	char body[MAX_ELEMENT_SIZE];
	size_t body_size;
	//const char *host;
	//const char *userinfo;
	//uint16_t port;
	int num_headers;
	enum { NONE=0, FIELD, VALUE } last_header_element;
	char headers [MAX_HEADERS][2][MAX_ELEMENT_SIZE];
	int should_keep_alive;

	char *upgrade; // upgraded body

	unsigned short http_major;
	unsigned short http_minor;

	int body_is_final;
} messages;

int message_begin_cb(http_parser *hp)
{
	printf("on_message_begin\n");
	return 0;
}
#if 0
int status_complete_cb(http_parser *hp){
	printf("on_status_complete\n");
	return 0;
}
#endif
int headers_complete_cb(http_parser *hp){
	printf("on_headers_complete\n");
	messages.method = hp->method;
	messages.status_code = hp->status_code;
	messages.http_major = hp->http_major;
	messages.http_minor = hp->http_minor;
	messages.should_keep_alive = http_should_keep_alive(hp);
	printf("%d,%d,%d,%d,%d\n", messages.method, messages.status_code, messages.http_major, messages.http_minor, messages.should_keep_alive);
	return 0;
}
int message_complete_cb(http_parser *hp){
	printf("on_message_complete\n");
	if (messages.should_keep_alive != http_should_keep_alive(hp)){
		fprintf(stderr, "\n\n *** Error http_should_keep_alive() should have same "
				"value in both on_message_complete and on_headers_complete "
				"but it doesn't! ***\n\n");
		assert(0);
		abort();
	}

	if (messages.body_size && http_body_is_final(hp) && !messages.body_is_final){
		fprintf(stderr, "\n\n *** Error http_body_is_final() should return 1 "
				"on last on_body callback call "
				"but it doesn't! ***\n\n");
		assert(0);
		abort();
	}
	messages.upgrade = 0x100;//TODO
	return 0;
}

int header_field_cb (http_parser *hp, const char *at, size_t length)
{
	printf("on_header_field\n");
	struct message *m = &messages;

	if (m->last_header_element != FIELD)
		m->num_headers++;

	strlncat(m->headers[m->num_headers-1][0],
			sizeof(m->headers[m->num_headers-1][0]),
			at,
			length);

	m->last_header_element = FIELD;

	return 0;
}

int header_value_cb (http_parser *hp, const char *at, size_t length)
{
	printf("on_header_value\n");
	struct message *m = &messages;

	strlncat(m->headers[m->num_headers-1][1],
			sizeof(m->headers[m->num_headers-1][1]),
			at,
			length);

	m->last_header_element = VALUE;
	return 0;
}
int url_cb (http_parser *hp, const char *at, size_t length)
{
	printf("on_url\n");
	printf("path from --> %s\n", at);
	strlncat(messages.request_url,
			sizeof(messages.request_url),
			at,
			length);

	return 0;
}
int body_cb (http_parser *hp, const char *at, size_t length)
{
	printf("on_body\n");
	strlncat(messages.body,
			sizeof(messages.body),
			at,
			length);
	messages.body_size += length;
	if (messages.body_is_final) {
		fprintf(stderr, "\n\n *** Error http_body_is_final() should return 1 "
				"on last on_body callback call "
				"but it doesn't! ***\n\n");
		assert(0);
		abort();
	}
	messages.body_is_final = http_body_is_final(hp);
	return 0;
}

static http_parser_settings settings =
{.on_message_begin = message_begin_cb
#if 0
	,.on_status_complete = status_complete_cb
#else
		,.on_status_complete = 0
#endif
		,.on_headers_complete = headers_complete_cb
		,.on_message_complete = message_complete_cb

		,.on_header_field = header_field_cb
		,.on_header_value = header_value_cb
		,.on_url = url_cb
		,.on_body = body_cb
};

#if 0
int main ()
{
#if 0
	char *data = "GET / HTTP/1.0\r\n" 
		"User-Agent: Wget/1.11.4\r\n" 
		"Accept: */*\r\n"
		"Host: www.163.com\r\n"
		"Connection: Keep-Alive\r\n"
		"\r\n";
#else
	char *data = "POST /data HTTP/1.0\r\n" 
		"User-Agent: Wget/1.11.4\r\n" 
		"Accept: */*\r\n"
		"Content-Length: 3\r\n"
		"Host: www.163.com\r\n"
		"Connection: Keep-Alive\r\n"
		"\r\n123";
#endif
	http_parser hp;

	hp.data = 0;
	http_parser_init(&hp, HTTP_REQUEST);

	int size = http_parser_execute(&hp, &settings, data, strlen(data));
	printf("%d of %d\n", size, strlen(data));
	printf("%d\n", hp.method);

	return 0;
}
#else
int main ()
{
	int i = 0;
	enum http_errno err;

	unsigned long version;
	unsigned major;
	unsigned minor;
	unsigned patch;

	version = http_parser_version();
	major = (version >> 16) & 255;
	minor = (version >> 8) & 255;
	patch = version & 255;
	printf("http_parser v%u.%u.%u (0x%06lx)\n", major, minor, patch, version);
#if 0
	char *head = "POST /da";
	char *body = "123456";
#endif
	//char *head = "POS";
	//char *body = "T ";
#if 0
	/data HTTP/1.1\r\n" 
		"User-Agent: Wget/1.11.4\r\n" 
		"Accept: */*\r\n"
		"Host: www.163.com\r\n"
		"Connection: close\r\n"
		"\r\n";
	char *body = "123456";
#endif
#if 1
	char *head = "POST /data HTTP/1.0\r\n" 
		"User-Agent: Wget/1.11.4\r\n" 
		"Accept: */*\r\n"
		"Content-Length: 6\r\n"
		"Host: www.163.com\r\n"
		"Connection: Keep-Alive\r\n"
		"\r\n";
	char *body = "123456";
#endif
#if 0
	char *head = "POST /data HTTP/1.0\r\nAcc";
	char *body = "ept: */*\r\n"
		"Host: www.163.com\r\n"
		"Content-Length: 6\r\n"
		"Connection: close\r\n"
		"\r\nsadkjk";
#endif
	http_parser hp;

	hp.data = 0;
	for (i=0;i<1;i++){
		http_parser_init(&hp, HTTP_REQUEST);
		//http_parser_settings settings = settings;

		int size = http_parser_execute(&hp, &settings, head, strlen(head));
		printf("%d of %d\n", size, strlen(head));
		printf("~~~%d %d\n", hp.method, hp.upgrade);
		//if (hp.upgrade && messages.upgrade) {
		if (hp.upgrade) {
			/* handle new protocol  处理新协议*/
		} else if (size != strlen(head)) {
			err = HTTP_PARSER_ERRNO(&hp);
			if (HPE_OK != err) {
				fprintf(stderr, "\n*** test_simple expected %s, but saw %s ***\n%s\n",
						http_errno_name(HPE_OK), http_errno_name(err), head);
				//http_errno_description(HPE_OK), http_errno_description(err), head);
			}
			/* Handle error. Usually just close the connection.  处理错误,通常是关闭这个连接*/
		}

		size = http_parser_execute(&hp, &settings, body, strlen(body));
		printf("%d of %d\n", size, strlen(body));
		printf("~~~%d %d\n", hp.method, hp.upgrade);
		//if (hp.upgrade && messages.upgrade) {
		if (hp.upgrade) {
			/* handle new protocol  处理新协议*/
		} else if (size != strlen(body)) {
			err = HTTP_PARSER_ERRNO(&hp);
			if (HPE_OK != err) {
				fprintf(stderr, "\n*** test_simple expected %s, but saw %s ***\n%s\n",
						http_errno_name(HPE_OK), http_errno_name(err), body);
				//http_errno_description(HPE_OK), http_errno_description(err), body);
			}
			/* Handle error. Usually just close the connection.  处理错误,通常是关闭这个连接*/
		}
#if 0
		size = http_parser_execute(&hp, &settings, "sajks", 5);
		printf("%d of %d\n", size, 5);
		printf("%d\n", hp.method);
		if (hp.upgrade && messages.upgrade) {
			/* handle new protocol  处理新协议*/
		} else if (size != strlen(body)) {
			err = HTTP_PARSER_ERRNO(&hp);
			if (HPE_OK != err) {
				fprintf(stderr, "\n*** test_simple expected %s, but saw %s ***\n%s\n",
						http_errno_name(HPE_OK), http_errno_name(err), "sajks");
				//http_errno_description(HPE_OK), http_errno_description(err), body);
			}
			/* Handle error. Usually just close the connection.  处理错误,通常是关闭这个连接*/
		}
#endif
	}

	return 0;
}
#endif
