#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#include "http.h"
#include "net_cache.h"
#include "major_def.h"

static char *x_strstr(const char *s1, const char *s2, int size)
{
	const char *p = s1;
	const size_t len = strlen (s2);
	const int idx = size - len;
	for (; ((p = strchr (p, *s2)) != 0) && ((p - s1) <= idx); p++)
	{
		if (strncmp (p, s2, len) == 0)
			return (char *)p;
	}
	return (0);
}

static size_t strlncat(char *dst, size_t len, const char *src, size_t n)
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
static int message_begin_cb(http_parser *hp)
{
	// x_printf("on_message_begin\n");
	struct data_node *p_node = hp->data;
	//-->On request,life only enter once.
	memset(&p_node->http_info.hs, 0, sizeof(struct http_status));
	return 0;
}
static int headers_complete_cb(http_parser *hp)
{
	// x_printf("on_headers_complete\n");
	struct data_node *p_node = hp->data;
	struct http_status *p_stat = &p_node->http_info.hs;

	p_stat->method = hp->method;
	p_stat->http_major = hp->http_major;
	p_stat->http_minor = hp->http_minor;
	p_stat->should_keep_alive = http_should_keep_alive(hp);
	// x_printf("%d,%d,%d,%d\n", p_stat->method, p_stat->http_major, p_stat->http_minor, p_stat->should_keep_alive);
	
	p_stat->path_offset = p_stat->url_offset;
	char *p_mark = x_strstr( (p_node->recv.buf_addr + p_stat->url_offset), "?", p_stat->url_len);
	if (p_mark){
		p_stat->path_len = p_mark - (p_node->recv.buf_addr + p_stat->path_offset);
		p_stat->uri_offset = p_mark + 1 - p_node->recv.buf_addr;
		p_stat->uri_len = p_stat->url_len - p_stat->path_len -1;
	}else{
		p_stat->path_len = p_stat->url_len;
		p_stat->uri_offset = 0;
		p_stat->uri_len = 0;
	}
	return 0;
}
static int message_complete_cb(http_parser *hp)
{
	// x_printf("on_message_complete\n");
	struct data_node *p_node = hp->data;
	struct http_status *p_stat = &p_node->http_info.hs;
	
	p_stat->over = TRUE;
	return 0;
}

static int header_field_cb (http_parser *hp, const char *at, size_t length)
{
	// x_printf("on_header_field\n");
	struct data_node *p_node = hp->data;
	struct http_status *p_stat = &p_node->http_info.hs;

	if (p_stat->last_header_element != FIELD)
		p_stat->num_headers++;
/*
	strlncat(m->headers[m->num_headers-1][0],
			sizeof(m->headers[m->num_headers-1][0]),
			at,
			length);
*/
	p_stat->last_header_element = FIELD;
	return 0;
}

static int header_value_cb (http_parser *hp, const char *at, size_t length)
{
	// x_printf("on_header_value\n");
	struct data_node *p_node = hp->data;
	struct http_status *p_stat = &p_node->http_info.hs;
/*
	strlncat(m->headers[m->num_headers-1][1],
			sizeof(m->headers[m->num_headers-1][1]),
			at,
			length);
*/
	p_stat->last_header_element = VALUE;
	return 0;
}
static int url_cb (http_parser *hp, const char *at, size_t length)
{
	// x_printf("on_url\n");
	struct data_node *p_node = hp->data;
	struct http_status *p_stat = &p_node->http_info.hs;
	
	if (p_stat->url_offset == 0){
		p_stat->url_offset = at - p_node->recv.buf_addr;
	}
	p_stat->url_len += length;
	return 0;
}
static int body_cb (http_parser *hp, const char *at, size_t length)
{
	// printf("on_body\n");
	struct data_node *p_node = hp->data;
	struct http_status *p_stat = &p_node->http_info.hs;
	
	if (p_stat->body_offset == 0){
		p_stat->body_offset = at - p_node->recv.buf_addr;
	}
	/*
	strlncat(messages.body,
			sizeof(messages.body),
			at,
			length);
	*/
	p_stat->body_size += length;
	return 0;
}

static http_parser_settings settings = {
	.on_message_begin = message_begin_cb
	,.on_status = 0
	,.on_headers_complete = headers_complete_cb
	,.on_message_complete = message_complete_cb

	,.on_header_field = header_field_cb
	,.on_header_value = header_value_cb
	,.on_url = url_cb
	,.on_body = body_cb
};


int http_handle(struct data_node *p_node)
{
	http_parser *hp = &p_node->http_info.hp;
	struct http_status *p_stat = &p_node->http_info.hs;
	if ((p_stat->dosize == 0) || (p_stat->step == 0)){
		http_parser_init(hp, HTTP_REQUEST);
	}
	int todo = (p_node->recv.get_size - p_stat->dosize);
	// printf("recv.get_size %d  dosize %d\n", p_node->recv.get_size, p_stat->dosize);
	int done = http_parser_execute(hp,
			&settings,
			(p_node->recv.buf_addr + p_stat->dosize),
			todo);
	p_stat->step++;
	if (p_stat->over) {
		p_stat->dosize = 0;
	}else{
		p_stat->dosize += done;
	}
	// x_printf("%d of %d\n", done, todo);

	if (hp->upgrade) {
		/* handle new protocol  处理新协议*/
		//TODO
		// x_printf("upgrade!\n");
		return FALSE;
	} else {
		if (done == todo) {
			return (p_stat->over);
		}else{
			/*it's error request,change to over and shoud broken socket*/
			/* Handle error. Usually just close the connection.  处理错误,通常是关闭这个连接*/
			p_stat->err = HTTP_PARSER_ERRNO(hp);
			if (HPE_OK != p_stat->err) {
				fprintf(stderr, "\n*** server expected %s, but saw %s ***\n%s\n",
						http_errno_name(HPE_OK), http_errno_name(p_stat->err), (p_node->recv.buf_addr + p_stat->dosize));
			}
			p_stat->dosize = 0;
			return TRUE;
		}
	}
}

int app_lua_get_head_data(lua_State *L)
{
	int fd = lua_tointeger(L, 1);
	struct data_node *p_node = get_pool_addr( fd );
	struct http_status *p_stat = &p_node->http_info.hs;
	if (p_stat->body_size == 0){
		lua_pushnil( L );
	}else{
		lua_pushlstring( L, (const char* )(p_node->recv.buf_addr), p_stat->body_offset - 2 );
	}
	return 1;
}

int app_lua_get_body_data(lua_State *L)
{
	int fd = lua_tointeger(L, 1);
	struct data_node *p_node = get_pool_addr( fd );
	struct http_status *p_stat = &p_node->http_info.hs;
	if (p_stat->body_size == 0){
		lua_pushnil( L );
	}else{
		lua_pushlstring( L, (const char* )(p_node->recv.buf_addr + p_stat->body_offset), p_stat->body_size );
	}
	return 1;
}

int app_lua_get_path_data(lua_State *L)
{
	int fd = lua_tointeger(L, 1);
	struct data_node *p_node = get_pool_addr( fd );
	struct http_status *p_stat = &p_node->http_info.hs;
	if (p_stat->path_len == 0){
		lua_pushnil( L );
	}else{
		lua_pushlstring( L, (const char* )(p_node->recv.buf_addr + p_stat->path_offset), p_stat->path_len );
	}
	return 1;
}

int app_lua_get_uri_args(lua_State *L)
{
	int fd = lua_tointeger(L, 1);
	struct data_node *p_node = get_pool_addr( fd );
	struct http_status *p_stat = &p_node->http_info.hs;
	if (p_stat->uri_len == 0){
		lua_pushnil( L );
	}else{
		lua_pushlstring( L, (const char* )(p_node->recv.buf_addr + p_stat->uri_offset), p_stat->uri_len );
	}
	return 1;
}

int app_lua_add_send_data(lua_State *L)
{
	size_t len = 0;
	int fd = lua_tointeger(L, 1);
	const char *data = lua_tolstring(L, 2, &len);

	if (data == NULL) {
		lua_pushboolean( L, 0 );
	}else{
		struct data_node *p_node = get_pool_addr( fd );
		int ok = cache_add(&p_node->send, data, len);
		if (ok == 0){
			lua_pushboolean( L, 1 );
		}else{
			lua_pushboolean( L, 0 );
		}
	}
	return 1;
}

char HTTP_RESP_INFO[][1024] = {
	"HTTP/1.1 200 OK\r\n"
	"Server: nginx/1.4.2\r\n"
	"Content-Type: text/html\r\n"
	"Connection: close\r\n"
	"\r\n",

	"HTTP/1.1 200 OK\r\n"
	"Server: nginx/1.4.2\r\n"
	"Content-Type: text/html\r\n"
	"Connection: Keep-Alive\r\n"
	"\r\n",

	"HTTP/1.1 400 Not Found\r\n"
	"Server: nginx/1.4.2\r\n"
	"Content-Type: text/html\r\n"
	"Content-Length: 168\r\n"
	"Connection: close\r\n"
	"\r\n"
	"<html>\r\n<head><title>404 Not Found</title></head>\r\n"
	"<body bgcolor=\"white\">\r\n<center><h1>404 Not Found</h1></center>\r\n"
	"<hr><center>nginx/1.4.2</center>\r\n</body>\r\n</html>\r\n",
	
	"HTTP/1.1 413 Request Entity Too Large\r\n"
	"Server: nginx/1.4.2\r\n"
	"Content-Type: text/html\r\n"
	"Content-Length: 198\r\n"
	"Connection: close\r\n"
	"\r\n"
	"<html>\r\n<head><title>413 Request Entity Too Large</title></head>\r\n"
	"<body bgcolor=\"white\">\r\n<center><h1>413 Request Entity Too Large</h1></center>\r\n"
	"<hr><center>nginx/1.4.2</center>\r\n</body>\r\n</html>\r\n",

	"HTTP/1.1 500 Internal Server Error\r\n"
	"Server: nginx/1.4.2\r\n"
	"Content-Type: text/html\r\n"
	"Content-Length: 192\r\n"
	"Connection: close\r\n"
	"\r\n"
	"<html>\r\n<head><title>500 Internal Server Error</title></head>\r\n"
	"<body bgcolor=\"white\">\r\n<center><h1>500 Internal Server Error</h1></center>\r\n"
	"<hr><center>nginx/1.4.2</center>\r\n</body>\r\n</html>\r\n",

	"HTTP/1.1 503 Service Unavailable\r\n"
	"Server: nginx/1.4.2\r\n"
	"Content-Type: text/html\r\n"
	"Content-Length: 180\r\n"
	"Connection: close\r\n"
	"\r\n"
	"<html>\r\n<head><title>503 Service Unavailable</title></head>\r\n"
	"<body bgcolor=\"white\">\r\n<center><h1>503 Service Unavailable</h1></center>\r\n"
	"<hr><center>nginx/1.4.2</center>\r\n</body>\r\n</html>\r\n"
};

void http_response(struct data_node *p_node, int code)
{
	if (p_node->send.get_size > 0){
		return;
	}
	switch (code) {
		case 200:
			cache_add(&p_node->send,
					HTTP_RESP_INFO[p_node->http_info.hs.should_keep_alive],
					strlen(HTTP_RESP_INFO[p_node->http_info.hs.should_keep_alive]));
			break;
		case 400:
			cache_add(&p_node->send,
					HTTP_RESP_INFO[ 2 ],
					strlen(HTTP_RESP_INFO[ 2 ]));
			break;
		case 413:
			cache_add(&p_node->send,
					HTTP_RESP_INFO[ 3 ],
					strlen(HTTP_RESP_INFO[ 3 ]));
			break;
		case 500:
			cache_add(&p_node->send,
					HTTP_RESP_INFO[ 4 ],
					strlen(HTTP_RESP_INFO[ 4 ]));
			break;
		default:
			cache_add(&p_node->send,
					HTTP_RESP_INFO[ 5 ],
					strlen(HTTP_RESP_INFO[ 5 ]));
			break;
	}
}


int app_lua_get_recv_buf(lua_State *L)
{
	int fd = lua_tointeger(L, 1);
	struct data_node *p_node = get_pool_addr( fd );
	unsigned int buf_len = 0;
	if(p_node != NULL)
		buf_len = p_node->recv.get_size;
	if (buf_len == 0 || p_node == NULL){
		lua_pushnil( L );
	}else{
		lua_pushlstring( L, (const char* )(p_node->recv.buf_addr), buf_len);
	}
	return 1;
}
