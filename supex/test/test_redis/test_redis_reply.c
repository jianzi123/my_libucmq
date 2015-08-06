#include "test_redis_reply.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>


static inline void _redis_reply_error(struct redis_reply *reply, int err, char *errmsg)
{
        reply->type = REDIS_REPLY_ERROR;
        reply->err = err;
        reply->str = strdup(errmsg);
        reply->len = strlen(errmsg);
}

static char* _redis_reply_next_value(const char *buf, size_t buf_len, char needle)
{
        int i;
        
        if (buf == NULL || buf_len == 0) return NULL;
        for (i = 0; i < buf_len; i++) {
                if (*(buf + i) == needle)
                        return (char*)(buf + i);
        }
        return NULL;
}

static char* _redis_reply_str_dup(const char *src, size_t count)
{
        char *buf;
        
        if (src == NULL || count == 0) return NULL;

        buf = malloc(count+1);
        if (!buf) return NULL;
        memcpy(buf, (void*)src, count);
        buf[count] = '\0';
        
        return buf;
}

/* Read a long long value starting at *s, under the assumption that it will be
 * terminated by \r\n. Ambiguously returns -1 for unexpected input. */
static long long _redis_reply_str_to_ll(const char *s)
{
        long long v = 0;
        int dec, mult = 1;
        char c;

        if (*s == '-') {
                mult = -1;
                s++;
        } else if (*s == '+') {
                mult = 1;
                s++;
        }

        while ((c = *(s++)) != '\r') {
                dec = c - '0';
                if (dec >= 0 && dec < 10) {
                        v *= 10;
                        v += dec;
                } else {
                        /* Should not happen... */
                        return -1;
                }
        }

        return mult*v;
}

void _redis_reply_proto_line(struct redis_reply *reply, const char *proto, size_t len)
{
        char *tmp, *cp;

        tmp = _redis_reply_next_value(proto, len, '\r');
        if (!tmp) {
                _redis_reply_error(reply, REDIS_ERROR_PROTOCOL, "Invalid redis protocol");
                return;
        }
        
        cp = _redis_reply_str_dup(proto+1, tmp-proto-1);
        reply->str = cp;
        reply->len = tmp-proto-1;        
        if (!reply->str)
                _redis_reply_error(reply, REDIS_ERROR_OTHER, "Invalid string dup!");
}

static void _redis_reply_proto_plus(struct redis_reply *reply, const char *proto, size_t len)
{
        reply->type = REDIS_REPLY_STATUS;
        _redis_reply_proto_line(reply, proto, len);
}

static void _redis_reply_proto_sub(struct redis_reply *reply, const char *proto, size_t len)
{
        reply->type = REDIS_REPLY_ERROR;
        _redis_reply_proto_line(reply, proto, len);
}

static void _redis_reply_proto_colon(struct redis_reply *reply, const char *proto, size_t len)
{
        reply->type = REDIS_REPLY_INTEGER;
        reply->integer = _redis_reply_str_to_ll(proto+1);
}

static void _redis_reply_proto_dollar(struct redis_reply *reply, const char *proto, size_t len)
{
        char *start, *cp;
        long long bytes;

        bytes = _redis_reply_str_to_ll(proto+1);
        if (bytes == -1) {
                reply->type = REDIS_REPLY_NIL;
                return;
        } else {
                reply->type = REDIS_REPLY_STRING;
        }
        
        start = _redis_reply_next_value(proto, len, '\n');
        cp = _redis_reply_str_dup(start+1, bytes);
        reply->str = cp;
        reply->len = bytes;
}

static void _redis_reply_proto_asterisk(struct redis_reply *reply, const char *proto, size_t len)
{
        int argv = 0;
        char *start, *cp;
        long long lines, bytes;

        
        reply->type = REDIS_REPLY_ARRAY;
        lines = _redis_reply_str_to_ll(proto+1);
        if (lines <= 0) { /** <= 0 treat as nil */
                reply->type = REDIS_REPLY_NIL;
                return;
        }

        start = _redis_reply_next_value(proto, len, '$');
        if (start == NULL) {
                _redis_reply_error(reply, REDIS_ERROR_PROTOCOL, "Can't match first '$'!");
                return;
        }

        while(lines--) {
                if (start == NULL) {
                        _redis_reply_error(reply, REDIS_ERROR_PROTOCOL, "Can't match next '$' from protocol!");
                        goto ERROR_EXIT;
                }
                start = start+1;
                bytes = _redis_reply_str_to_ll(start);
                if (bytes == -1) {
                        _redis_reply_error(reply, REDIS_ERROR_PROTOCOL, "Can't get long long type from protocol!");
                        goto ERROR_EXIT;
                }
                
                start = _redis_reply_next_value(start, len-(start-proto+1),'\n');
                if (start == NULL) {
                        _redis_reply_error(reply, REDIS_ERROR_PROTOCOL, "Can't match next '\\n' from protocol!");
                        goto ERROR_EXIT; 
                }

                start = start+1;
                cp = _redis_reply_str_dup(start, bytes);
                if (cp == NULL) {
                        _redis_reply_error(reply, REDIS_ERROR_PROTOCOL, "Copy string failed from protocol!");
                        goto ERROR_EXIT; 
                }
                /** store to element */
                reply->elements = argv+1;
                reply->element = realloc(reply->element, reply->elements*sizeof(struct redis_reply*));
                reply->element[argv] = calloc(1, sizeof(struct redis_reply));
                reply->element[argv]->str = cp;
                reply->element[argv++]->len = bytes;

                start = _redis_reply_next_value(start, len-(start-proto+1), '$');
        }
        return;

ERROR_EXIT:
        /** free filled element array */
        while(argv--) {
                free(reply->element[argv]);
        }
        
        if (reply->element != NULL)
                free(reply->element);

        return;
}

/**
 * Check protocol format.
 */
int first_reply_ok(const char *buf, size_t buflen, size_t *reply_len)
{
        char *first, *second;
        const char *cur;
        long long lines, bytes;
        
        if (!buf || buflen < 4) /** [ (+|-|*|$|:) + (content) + (\r\n) ] */
                goto ERROR_EXIT;
        
        switch(buf[0]) {
        case '+':
        case '-':
                cur = strstr(buf, "\r\n");
                if (!cur) goto ERROR_EXIT;
                if (reply_len) *reply_len = cur-buf+2;
                return 1;
        case ':':
                bytes = _redis_reply_str_to_ll(buf+1);

                cur = strstr(buf, "\r\n");
                if (!cur) goto ERROR_EXIT;
                
                if (reply_len) *reply_len = cur-buf+2;
                return 1;
        case '$':
                bytes = _redis_reply_str_to_ll(buf+1);
                first = strstr(buf, "\r\n");
                if (bytes == -1) {
                        if (!first) goto ERROR_EXIT;
                        if (reply_len) *reply_len = 5; /** $-1\r\n */
                        return 1;
                } else {
                        second = strstr(first+2, "\r\n");
                        if (!second) goto ERROR_EXIT;
                        if (reply_len) *reply_len = second-buf+2;
                        return 1;
                }
        case '*':
                cur = buf;
                lines = _redis_reply_str_to_ll(cur+1);
                if (lines <= 0) {
                        cur = strstr(cur, "\r\n");
                        if (!cur) goto ERROR_EXIT;
                        if (reply_len) *reply_len = cur-buf+2;
                        return 1;
                }
                 
                while(lines--) {
                        cur = strstr(cur, "\r\n");
                        if (!cur) goto ERROR_EXIT;
                        cur = cur+2; // pointer to first '$'
                        
                        if (cur[0] != '$') goto ERROR_EXIT;
                        bytes = _redis_reply_str_to_ll(cur+1);
                        if (bytes == -1) goto ERROR_EXIT;

                        cur = strstr(cur, "\r\n");
                        if (!cur) goto ERROR_EXIT;
                        
                        second = strstr(cur+2, "\r\n");
                        if (!second) goto ERROR_EXIT;
                        if (second-cur-2 != bytes) goto ERROR_EXIT;

                        cur = second;
                }
                if (reply_len) *reply_len = cur-buf+2;
                return 1;
        }

ERROR_EXIT:
        if (reply_len) *reply_len = 0;
        return 0;
}

struct redis_reply* redis_reply_create()
{
        struct redis_reply *reply;

        reply = calloc(1, sizeof(*reply));
        assert(reply);
        
        return reply;
}

void redis_reply_release(struct redis_reply *reply)
{
        int i;
        if (!reply) return;

        switch(reply->type) {
        case REDIS_REPLY_NIL:
        case REDIS_REPLY_INTEGER:
                break;
        case REDIS_REPLY_ERROR:
        case REDIS_REPLY_STRING:
        case REDIS_REPLY_STATUS:
                if (reply->str) free(reply->str);
                break;
        case REDIS_REPLY_ARRAY:
                for (i = 0; i < reply->elements; i++) {
                        free(reply->element[i]);
                }
                if (reply->element)
                        free(reply->element);
                break;
        }

        free(reply);
}

static struct redis_reply* _proto_to_reply(const char *proto, size_t len)
{
        struct redis_reply *reply;

        reply = redis_reply_create();

        if (proto == NULL || len == 0) {
                _redis_reply_error(reply, REDIS_ERROR_OOM, "No more memory available!");
                return reply;
        }
        if (proto[len-2] != '\r' && proto[len-1] != '\n') {
                _redis_reply_error(reply, REDIS_ERROR_PROTOCOL, "Invalid redis protocol");
                return reply;
        }

        switch(proto[0]) {
        case '+':
                _redis_reply_proto_plus(reply, proto, len);
                return reply;
        case '-':
                _redis_reply_proto_sub(reply, proto, len);
                return reply;
        case ':':
                _redis_reply_proto_colon(reply, proto, len);
                return reply;
        case '$':
                _redis_reply_proto_dollar(reply, proto, len);
                return reply;
        case '*':
                _redis_reply_proto_asterisk(reply, proto, len);
                return reply;
        default:
                _redis_reply_error(reply, REDIS_ERROR_PROTOCOL, "Invalid redis protocol");
                return reply;
        }

        return reply;
}

/**
 * Convert redis protocol to redis_reply structure.
 */
struct redis_reply* proto_to_reply(const char *proto, size_t proto_len)
{
        return _proto_to_reply(proto, proto_len);
}

