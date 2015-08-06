#include "check_parments.h"
int upper_to_lower(char *p)
{
	char *tmp = p;
	while(*p!='\0'){
		*p = tolower(*p);
		p++;
		if(*p == ':'){
			if ( p-tmp == 4)
				break;
			else
				return -1;
		}
	}
	return 1;
}

void  create_uuid(char *L)
{
	char str[37] = {0};
	char *p = str;
	uuid_t uuid;

	uuid_generate_time(uuid);
	uuid_unparse(uuid, str);
	while(*p!='\0'){
		if (*p=='-')
			strcpy(p,p+1);
		else
			p++;
	}
	strcat(L, str);
}

static int check_imei(char *p)
{
	while( *p != '\0' ){
		if (*p>='0' && *p<='9')
			p++;
		else
			return WEIBO_FAILED;	
	}
	return WEIBO_SUCESS;
}

static int check_accountID(char *p)
{
	while( *p != '\0' ){
		switch(*p){
			case '0' ... '9':
				break;
			case 'a' ... 'z':
				break;
			case 'A' ... 'Z':
				break;
			default:
				return WEIBO_FAILED;
		}
		p++;	
	}
	return WEIBO_SUCESS;
}
int check_user(char *p)
{
	if(p){
		int len = strlen(p);
		if (len == 15){
			return check_imei(p);
		}
		if (len == 10){
			return check_accountID(p);
		}
	}
	return WEIBO_FAILED;
}



void content_unescape_uri(u_char **dst, u_char **src, size_t size,int type)
{
    u_char  *d, *s, ch, c, decoded;
    enum {
        sw_usual = 0,
        sw_quoted,
        sw_quoted_second
    } state;

    d = *dst;
    s = *src;

    state = 0;
    decoded = 0;

    while (size--) {

        ch = *s++;

        switch (state) {
        case sw_usual:
            if (ch == '?'
                && (type & (NGX_UNESCAPE_URI|NGX_UNESCAPE_REDIRECT)))
            {
                *d++ = ch;
                goto done;
            }

            if (ch == '%') {
                state = sw_quoted;
                break;
            }

            *d++ = ch;
            break;

        case sw_quoted:

            if (ch >= '0' && ch <= '9') {
                decoded = (u_char) (ch - '0');
                state = sw_quoted_second;
                break;
            }

            c = (u_char) (ch | 0x20);
            if (c >= 'a' && c <= 'f') {
                decoded = (u_char) (c - 'a' + 10);
                state = sw_quoted_second;
                break;
            }

            /* the invalid quoted character */

            state = sw_usual;

            *d++ = ch;

            break;

        case sw_quoted_second:

            state = sw_usual;

            if (ch >= '0' && ch <= '9') {
                ch = (u_char) ((decoded << 4) + ch - '0');

                if (type & NGX_UNESCAPE_REDIRECT) {
                    if (ch > '%' && ch < 0x7f) {
                        *d++ = ch;
                        break;
                    }

                    *d++ = '%'; *d++ = *(s - 2); *d++ = *(s - 1);

                    break;
                }

                *d++ = ch;

                break;
            }

            c = (u_char) (ch | 0x20);
            if (c >= 'a' && c <= 'f') {
                ch = (u_char) ((decoded << 4) + c - 'a' + 10);

                if (type & NGX_UNESCAPE_URI) {
                    if (ch == '?') {
                        *d++ = ch;
                        goto done;
                    }

                    *d++ = ch;
                    break;
                }

                if (type & NGX_UNESCAPE_REDIRECT) {
                    if (ch == '?') {
                        *d++ = ch;
                        goto done;
                    }

                    if (ch > '%' && ch < 0x7f) {
                        *d++ = ch;
                        break;
                    }

                    *d++ = '%'; *d++ = *(s - 2); *d++ = *(s - 1);
                    break;
                }

                *d++ = ch;

                break;
            }

            /* the invalid quoted character */

            break;
        }
    }

done:

    *dst = d;
    *src = s;
}


int content_decode(u_char *dst, u_char *src)
{
	if (NULL == src || NULL == dst){
		return WEIBO_FAILED;
	}

	size_t len = strlen((const char *)src);
	content_unescape_uri(&dst, &src, len, NGX_UNESCAPE_URI_COMPONENT);
	return WEIBO_SUCESS;
}

