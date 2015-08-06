#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#include "http.h"
#include "async_api.h"
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
	struct async_ctx *p_node = hp->data;
	//-->On request,life only enter once.
	memset(&p_node->parse.http_info.hs, 0, sizeof(struct http_status));
	return 0;
}
int status_complete_cb(http_parser *hp){
	// x_printf("on_status_complete\n");
	struct async_ctx *p_node = hp->data;
	struct http_status *p_stat = &p_node->parse.http_info.hs;

	p_stat->status_code = hp->status_code;
	return 0;
}
extern int http_get_body_length(char *buf);
static int headers_complete_cb(http_parser *hp)
{
	// x_printf("on_headers_complete\n");
	struct async_ctx *p_node = hp->data;
	struct http_status *p_stat = &p_node->parse.http_info.hs;

	p_stat->method = hp->method;
	p_stat->http_major = hp->http_major;
	p_stat->http_minor = hp->http_minor;
	p_stat->should_keep_alive = http_should_keep_alive(hp);
	// x_printf("%d,%d,%d,%d\n", p_stat->method, p_stat->http_major, p_stat->http_minor, p_stat->should_keep_alive);
	
	p_stat->path_offset = p_stat->url_offset;
	char *p_mark = x_strstr( (p_node->replies.work->cache.buf_addr + p_stat->url_offset), "?", p_stat->url_len);
	if (p_mark){
		p_stat->path_len = p_mark - (p_node->replies.work->cache.buf_addr + p_stat->path_offset);
		p_stat->uri_offset = p_mark + 1 - p_node->replies.work->cache.buf_addr;
		p_stat->uri_len = p_stat->url_len - p_stat->path_len -1;
	}else{
		p_stat->path_len = p_stat->url_len;
		p_stat->uri_offset = 0;
		p_stat->uri_len = 0;
	}
	int size = http_get_body_length( p_node->replies.work->cache.buf_addr );
	if (size == 0){
		p_stat->over = TRUE;
	}
	return 0;
}
static int message_complete_cb(http_parser *hp)
{
	// x_printf("on_message_complete\n");
	struct async_ctx *p_node = hp->data;
	struct http_status *p_stat = &p_node->parse.http_info.hs;
	
	p_stat->over = TRUE;
	return 0;
}

static int header_field_cb (http_parser *hp, const char *at, size_t length)
{
	// x_printf("on_header_field\n");
	struct async_ctx *p_node = hp->data;
	struct http_status *p_stat = &p_node->parse.http_info.hs;

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
	struct async_ctx *p_node = hp->data;
	struct http_status *p_stat = &p_node->parse.http_info.hs;
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
	struct async_ctx *p_node = hp->data;
	struct http_status *p_stat = &p_node->parse.http_info.hs;
	
	if (p_stat->url_offset == 0){
		p_stat->url_offset = at - p_node->replies.work->cache.buf_addr;
	}
	p_stat->url_len += length;
	return 0;
}
static int body_cb (http_parser *hp, const char *at, size_t length)
{
	// printf("on_body\n");
	struct async_ctx *p_node = hp->data;
	struct http_status *p_stat = &p_node->parse.http_info.hs;
	
	if (p_stat->body_offset == 0){
		p_stat->body_offset = at - p_node->replies.work->cache.buf_addr;
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
	,.on_status = status_complete_cb
	,.on_headers_complete = headers_complete_cb
	,.on_message_complete = message_complete_cb

	,.on_header_field = header_field_cb
	,.on_header_value = header_value_cb
	,.on_url = url_cb
	,.on_body = body_cb
};


int http_client_handle(struct async_ctx *p_node)
{
	http_parser *hp = &p_node->parse.http_info.hp;
	struct http_status *p_stat = &p_node->parse.http_info.hs;
	if ((p_stat->dosize == 0) || (p_stat->step == 0)){
		http_parser_init(hp, HTTP_RESPONSE);
	}
	int todo = (p_node->replies.work->cache.get_size - p_stat->dosize);
	// printf("recv.get_size %d  dosize %d\n", p_node->replies.work->cache.get_size, p_stat->dosize);
	int done = http_parser_execute(hp,
			&settings,
			(p_node->replies.work->cache.buf_addr + p_stat->dosize),
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
						http_errno_name(HPE_OK), http_errno_name(p_stat->err), (p_node->replies.work->cache.buf_addr + p_stat->dosize));
			}
			p_stat->dosize = 0;
			return TRUE;
		}
	}
}

