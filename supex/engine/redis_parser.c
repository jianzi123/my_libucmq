#include <string.h>
#include <stdlib.h>

#include "sstr.h"
#include "redis_parser.h"
#include <ctype.h>
#include "utils.h"
#include "major_def.h"


static inline char *x_strstr(const char *s1, const char *s2, int size)
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


/*------------------------------api-------------------------------------------*/
int rds_reset_parser(struct data_node *p_node)
{
	memset( &p_node->redis_info.rp, 0, sizeof(struct redis_parser) );
	return 0;
}

static inline int rds_reset_status(struct data_node *p_node)
{
	memset( &p_node->redis_info.rs, 0, sizeof(struct redis_status) );
	return 0;
}

/*
 *	------------------------data-----------------------
 *	*3\r\n$3\r\nSET\r\n$5\r\nmykey\r\n$7\r\nmyvalue\r\n
 *	---------------------------------------------------
 */

/************W***************/
#define CMD_SET		12291
#define CMD_DEL		2143
#define CMD_MSET	223203
#define CMD_HSET	135323
#define CMD_LPUSHX	137913669
#define CMD_RPUSHX	209201925
/************R***************/
#define CMD_GET		4179
#define CMD_HMGET	3413923
#define CMD_HGETALL 0x85448761	
#define CMD_LRANGE	5325872
/************O***************/
#define CMD_QUIT	294963

/************P***************/
#define CHECK_BUFFER(x, y) do { \
	if (  (( offset = (p_node->recv.buf_addr + p_node->recv.get_size - (x + y)) ) <=  0)  ||  !(x = x_strstr( (x + y), "\r\n", offset)) ) { return X_DATA_NO_ALL; } \
	else { x = x + 2; } \
} while (0)


#define PARSE_LEN(x) do { if (x == 0){ \
	if (data[ p_parser->doptr ] != '$'){ \
		return X_PARSE_ERROR; \
	} \
	p_new = p_old = &data[ p_parser->doptr + 1 ]; \
	CHECK_BUFFER(p_new, 0); \
	x = atoi( p_old ); \
	p_parser->doptr = p_new - data; } \
} while (0)
#define PARSE_MEMBER(x,y) do { if (x == 0){ \
	p_new = p_old = &data[ p_parser->doptr ]; \
	CHECK_BUFFER(p_new, y); \
	x = p_old - data; \
	p_parser->doptr = p_new - data; } \
} while (0)
#define PARSE_MEMBER_AND_STORE(x, y, ss, sx, sy ) do { if (x == 0){ \
	p_new = p_old = &data[ p_parser->doptr ]; \
	CHECK_BUFFER(p_new, y); \
	x = p_old - data; \
	p_parser->doptr = p_new - data; \
	\
	if ( ss > MAX_KV_PAIRS_INDEX ) { return X_KV_TOO_MUCH; } \
	sx[ ss ] = x; \
	sy[ ss ] = y; \
	ss ++; } \
} while (0)




int rds_cmd_parse(struct data_node *p_node)
{
	int loop = 0;
	int i,j = 0;
	int offset = 0;
	size_t mlen = 0;
	char *p_new = NULL;
	char *p_old = NULL;

	/* parse cmd */
	char *data = p_node->recv.buf_addr;
	struct redis_parser *p_parser = &p_node->redis_info.rp;
	struct redis_status *p_status = &p_node->redis_info.rs;
	x_printf(D, "%s\n", data);
	if (data[0] == '*'){
		// 1. read the arg count:
		if (p_parser->kvs == 0) {
			p_new = p_old = &data[1];
			CHECK_BUFFER(p_new, 0);
			p_parser->kvs = atoi( p_old );
			p_parser->doptr = p_new - data;
			
			rds_reset_status(p_node);
		}
		// 2. read the request name
		if (p_parser->cmd == 0){
			if (data[ p_parser->doptr ] != '$'){
				return X_PARSE_ERROR;
			}
			p_new = p_old = &data[ p_parser->doptr + 1];
			CHECK_BUFFER(p_new, 0);
			mlen = atoi( p_old );
			p_old = p_new;
			CHECK_BUFFER(p_new, mlen);
			for (i = 0; i < mlen; i++){
				if ( *(p_old + i) > 'Z' ){
					*(p_old + i) -= 32;/* 'A' - 'a' */
				}
				p_parser->cmd = p_parser->cmd * 26 + ( *(p_old + i) - 'A' );/* A~Z is 26 numbers */
			}
			p_parser->doptr = p_new - data;
		}
		x_printf(D, "%d\n", p_parser->cmd);
		// 3. read a arg
		switch (p_parser->cmd){
			case CMD_SET:
				PARSE_LEN(p_parser->klen);
				PARSE_MEMBER_AND_STORE(p_parser->key, p_parser->klen,
						p_status->keys, p_status->key_offset, p_status->klen_array);
				PARSE_LEN(p_parser->vlen);
				PARSE_MEMBER_AND_STORE(p_parser->val, p_parser->vlen,
						p_status->vals, p_status->val_offset, p_status->vlen_array);

				p_status->order = SET_FUNC_ORDER;
				return X_DATA_IS_ALL;

			case CMD_LPUSHX:
				PARSE_LEN(p_parser->klen);
				PARSE_MEMBER_AND_STORE(p_parser->key, p_parser->klen,
						p_status->keys, p_status->key_offset, p_status->klen_array);
				PARSE_LEN(p_parser->vlen);
				PARSE_MEMBER_AND_STORE(p_parser->val, p_parser->vlen,
						p_status->vals, p_status->val_offset, p_status->vlen_array);

				p_status->order = LPUSHX_FUNC_ORDER;
				return X_DATA_IS_ALL; 

			case CMD_RPUSHX:
				PARSE_LEN(p_parser->klen);
				PARSE_MEMBER_AND_STORE(p_parser->key, p_parser->klen,
						p_status->keys, p_status->key_offset, p_status->klen_array);
				PARSE_LEN(p_parser->vlen);
				PARSE_MEMBER_AND_STORE(p_parser->val, p_parser->vlen,
						p_status->vals, p_status->val_offset, p_status->vlen_array);

				p_status->order = RPUSHX_FUNC_ORDER;
				return X_DATA_IS_ALL; 

			case CMD_DEL:
				PARSE_LEN(p_parser->klen);
				PARSE_MEMBER_AND_STORE(p_parser->key, p_parser->klen,
						p_status->keys, p_status->key_offset, p_status->klen_array);

				//p_status->type = BIT8_TASK_TYPE_ALONE;
				//p_status->func = NULL;//FIXME
				return X_DATA_IS_ALL; 

			case CMD_MSET:/* use one thread to write, otherwise add a mutex lock */
				loop = (p_parser->kvs - 1) / 2;
				for (j = 0; j < loop; j++){
					PARSE_LEN(p_parser->klen);
					PARSE_MEMBER_AND_STORE(p_parser->key, p_parser->klen,
							p_status->keys, p_status->key_offset, p_status->klen_array);
					PARSE_LEN(p_parser->vlen);
					PARSE_MEMBER_AND_STORE(p_parser->val, p_parser->vlen,
							p_status->vals, p_status->val_offset, p_status->vlen_array);

					p_parser->kvs -= 2;
					p_parser->klen = 0;
					p_parser->key = 0;
					p_parser->vlen = 0;
					p_parser->val = 0;
				}
				//p_status->type = BIT8_TASK_TYPE_ALONE;
				//p_status->func = NULL;//FIXME
				return X_DATA_IS_ALL; 
			
			case CMD_HSET:
				PARSE_LEN(p_parser->klen);
				PARSE_MEMBER_AND_STORE(p_parser->key, p_parser->klen,
						p_status->keys, p_status->key_offset, p_status->klen_array);
				PARSE_LEN(p_parser->flen);
				PARSE_MEMBER_AND_STORE(p_parser->fld, p_parser->flen,
						p_status->flds, p_status->fld_offset, p_status->flen_array);
				PARSE_LEN(p_parser->vlen);
				PARSE_MEMBER_AND_STORE(p_parser->val, p_parser->vlen,
						p_status->vals, p_status->val_offset, p_status->vlen_array);
				p_status->order = HSET_FUNC_ORDER;
				return X_DATA_IS_ALL; 

			case CMD_GET:
				PARSE_LEN(p_parser->klen);
				PARSE_MEMBER_AND_STORE(p_parser->key, p_parser->klen,
						p_status->keys, p_status->key_offset, p_status->klen_array);

				//p_status->type = BIT8_TASK_TYPE_ALONE;
				//p_status->func = NULL;//FIXME
				return X_DATA_IS_ALL; 
			//case CMD_MGET:
			case CMD_HMGET://FIXME key switch filed
				PARSE_LEN(p_parser->flen);
				PARSE_MEMBER_AND_STORE(p_parser->fld, p_parser->flen,
						p_status->flds, p_status->fld_offset, p_status->flen_array);

				loop = p_parser->kvs - 2;
				for (j = 0; j < loop; j++){
					PARSE_LEN(p_parser->klen);
					PARSE_MEMBER_AND_STORE(p_parser->key, p_parser->klen,
							p_status->keys, p_status->key_offset, p_status->klen_array);

					p_parser->kvs --;
					p_parser->klen = 0;
					p_parser->key = 0;
				}
				p_status->order = HMGET_FUNC_ORDER;
				return X_DATA_IS_ALL; 

			case CMD_HGETALL:
				PARSE_LEN(p_parser->klen);
				PARSE_MEMBER_AND_STORE(p_parser->key, p_parser->klen,
						p_status->keys, p_status->key_offset, p_status->klen_array);

				p_status->order = HGETALL_FUNC_ORDER;
				//p_status->type = BIT8_TASK_TYPE_ALONE;
				//p_status->func = NULL;//FIXME
				return X_DATA_IS_ALL; 

			case CMD_LRANGE:
				/* look as start key */
				PARSE_LEN(p_parser->klen);
				PARSE_MEMBER_AND_STORE(p_parser->key, p_parser->klen,
						p_status->keys, p_status->key_offset, p_status->klen_array);
				/* look as end key */
				PARSE_LEN(p_parser->vlen);
				PARSE_MEMBER_AND_STORE(p_parser->val, p_parser->vlen,
						p_status->vals, p_status->val_offset, p_status->vlen_array);

				//p_status->type = BIT8_TASK_TYPE_ALONE;
				//p_status->func = NULL;//FIXME
				return X_DATA_IS_ALL; 

			case CMD_QUIT:
				return X_REQUEST_QUIT;

			default:
				return X_REQUEST_ERROR;
		}
	}
	else if (data[0] == '$'){
#if 0
		// 1. read the request name
		if (p_parser->cmd == 0){
			p_new = p_old = &data[1];
			CHECK_BUFFER(p_new, 0);
			mlen = atoi( p_old );
			p_old = p_new;
			CHECK_BUFFER(p_new, mlen);
			for (i = 0; i < mlen; i++){
				p_parser->cmd = p_parser->cmd * 26 + ( *(p_old + i) - 'A' );/* A~Z is 26 numbers */
			}
			p_parser->doptr = p_new - data;
		}
		x_printf(D, "%d\n", p_parser->cmd);
		// 2. read a arg
		switch (p_parser->cmd){
			case CMD_QUIT:
				return CLIENT_QUIT;
			default:
		x_printf(D, "-----------------------------------------------------\n");
				return -1;
		}

	//TODO
#endif
		return X_REQUEST_ERROR;
	}else{
		return X_PARSE_ERROR;
	}
	return X_DATA_IS_ALL; 
}


/* Calculate the number of bytes needed to represent an integer as string. */
static int intlen(int i) {
    int len = 0;
    if (i < 0) {
        len++;
        i = -i;
    }
    do {
        len++;
        i /= 10;
    } while(i);
    return len;
}

/* Helper that calculates the bulk length given a certain string length. */
static size_t bulklen(size_t len) {
    return 1+intlen(len)+2+len+2;
}

static int _cmd_to_proto(char **target, const char *format, va_list ap) {
    const char *c = format;
    char *cmd = NULL; /* final command */
    int pos; /* position in final command */
    sstr curarg, newarg;
    int touched = 0; /* was the current argument touched? */
    char **curargv = NULL, **newargv = NULL;
    int argc = 0;
    int totlen = 0;
    int j;

    /* Abort if there is not target to set */
    if (target == NULL)
        return -1;

    /* Build the command string accordingly to protocol */
    curarg = sstr_empty();
    if (curarg == NULL)
        return -1;

    while(*c != '\0') {
        if (*c != '%' || c[1] == '\0') {
            if (*c == ' ') {
                if (touched) {
                    newargv = realloc(curargv,sizeof(char*)*(argc+1));
                    if (newargv == NULL) goto err;
                    curargv = newargv;
                    curargv[argc++] = curarg;
                    totlen += bulklen(sstr_len(curarg));

                    /* curarg is put in argv so it can be overwritten. */
                    curarg = sstr_empty();
                    if (curarg == NULL) goto err;
                    touched = 0;
                }
            } else {
                    newarg = sstr_catlen(curarg,c,1);
                if (newarg == NULL) goto err;
                curarg = newarg;
                touched = 1;
            }
        } else {
            char *arg;
            size_t size;

            /* Set newarg so it can be checked even if it is not touched. */
            newarg = curarg;

            switch(c[1]) {
            case 's':
                arg = va_arg(ap,char*);
                size = strlen(arg);
                if (size > 0)
                        newarg = sstr_catlen(curarg,arg,size);
                break;
            case 'b':
                arg = va_arg(ap,char*);
                size = va_arg(ap,size_t);
                if (size > 0)
                        newarg = sstr_catlen(curarg,arg,size);
                break;
            case '%':
                    newarg = sstr_cat(curarg,"%"); 
                break;
            default:
                /* Try to detect printf format */
                {
                    static const char intfmts[] = "diouxX";
                    char _format[16];
                    const char *_p = c+1;
                    size_t _l = 0;
                    va_list _cpy;

                    /* Flags */
                    if (*_p != '\0' && *_p == '#') _p++;
                    if (*_p != '\0' && *_p == '0') _p++;
                    if (*_p != '\0' && *_p == '-') _p++;
                    if (*_p != '\0' && *_p == ' ') _p++;
                    if (*_p != '\0' && *_p == '+') _p++;

                    /* Field width */
                    while (*_p != '\0' && isdigit(*_p)) _p++;

                    /* Precision */
                    if (*_p == '.') {
                        _p++;
                        while (*_p != '\0' && isdigit(*_p)) _p++;
                    }

                    /* Copy va_list before consuming with va_arg */
                    va_copy(_cpy,ap);

                    /* Integer conversion (without modifiers) */
                    if (strchr(intfmts,*_p) != NULL) {
                        va_arg(ap,int);
                        goto fmt_valid;
                    }

                    /* Double conversion (without modifiers) */
                    if (strchr("eEfFgGaA",*_p) != NULL) {
                        va_arg(ap,double);
                        goto fmt_valid;
                    }

                    /* Size: char */
                    if (_p[0] == 'h' && _p[1] == 'h') {
                        _p += 2;
                        if (*_p != '\0' && strchr(intfmts,*_p) != NULL) {
                            va_arg(ap,int); /* char gets promoted to int */
                            goto fmt_valid;
                        }
                        goto fmt_invalid;
                    }

                    /* Size: short */
                    if (_p[0] == 'h') {
                        _p += 1;
                        if (*_p != '\0' && strchr(intfmts,*_p) != NULL) {
                            va_arg(ap,int); /* short gets promoted to int */
                            goto fmt_valid;
                        }
                        goto fmt_invalid;
                    }

                    /* Size: long long */
                    if (_p[0] == 'l' && _p[1] == 'l') {
                        _p += 2;
                        if (*_p != '\0' && strchr(intfmts,*_p) != NULL) {
                            va_arg(ap,long long);
                            goto fmt_valid;
                        }
                        goto fmt_invalid;
                    }

                    /* Size: long */
                    if (_p[0] == 'l') {
                        _p += 1;
                        if (*_p != '\0' && strchr(intfmts,*_p) != NULL) {
                            va_arg(ap,long);
                            goto fmt_valid;
                        }
                        goto fmt_invalid;
                    }

                fmt_invalid:
                    va_end(_cpy);
                    goto err;

                fmt_valid:
                    _l = (_p+1)-c;
                    if (_l < sizeof(_format)-2) {
                        memcpy(_format,c,_l);
                        _format[_l] = '\0';
                        newarg = sstr_catvprintf(curarg,_format,_cpy);

                        /* Update current position (note: outer blocks
                         * increment c twice so compensate here) */
                        c = _p-1;
                    }

                    va_end(_cpy);
                    break;
                }
            }

            if (newarg == NULL) goto err;
            curarg = newarg;

            touched = 1;
            c++;
        }
        c++;
    }

    /* Add the last argument if needed */
    if (touched) {
        newargv = realloc(curargv,sizeof(char*)*(argc+1));
        if (newargv == NULL) goto err;
        curargv = newargv;
        curargv[argc++] = curarg;
        totlen += bulklen(sstr_len(curarg));
    } else {
            sstr_free(curarg);
    }

    /* Clear curarg because it was put in curargv or was free'd. */
    curarg = NULL;

    /* Add bytes needed to hold multi bulk count */
    totlen += 1+intlen(argc)+2;

    /* Build the command at protocol level */
    cmd = malloc(totlen+1);
    if (cmd == NULL) goto err;

    pos = sprintf(cmd,"*%d\r\n",argc);
    for (j = 0; j < argc; j++) {
            pos += sprintf(cmd+pos,"$%zu\r\n",sstr_len(curargv[j]));
            memcpy(cmd+pos,curargv[j],sstr_len(curargv[j]));
            pos += sstr_len(curargv[j]);
            sstr_free(curargv[j]);
        cmd[pos++] = '\r';
        cmd[pos++] = '\n';
    }
    assert(pos == totlen);
    cmd[pos] = '\0';

    free(curargv);
    *target = cmd;
    return totlen;

err:
    while(argc--)
            sstr_free(curargv[argc]);
    free(curargv);

    if (curarg != NULL)
            sstr_free(curarg);

    /* No need to check cmd since it is the last statement that can fail,
     * but do it anyway to be as defensive as possible. */
    if (cmd != NULL)
        free(cmd);

    return -1;
}

/**
 * @param  proto out parameter holds protocol string, it's your responsibility to free it. 
 * @return -1 if failed
 *         >0 indicates protocol bytes count converted.
 */
int cmd_to_proto(char **proto, const char *fmt, ...)
{
        int ret;
        va_list ap;

        va_start(ap, fmt);
        ret = _cmd_to_proto(proto, fmt, ap);
        va_end(ap);

        return ret;
}

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

