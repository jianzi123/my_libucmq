/**
 * @brief Structure string unit test by shishengjie
 */

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "test_sstr.h"
#include <assert.h>


        
sstr sstr_new_len(const void *init, size_t len)
{
        struct sstr_hdr *ss;

        if (init) {
                ss = malloc(sizeof(struct sstr_hdr) + len + 1);
        } else {
                ss = calloc(1, sizeof(struct sstr_hdr) + len + 1);
        }
        if (ss == NULL) return NULL;
        ss->free = 0;
        ss->len = len;
        if (init != NULL && len > 0)
                memcpy(ss->buf, init, len);
        ss->buf[ss->len] = '\0';
        return (sstr)ss->buf;
}

sstr sstr_new(const char *str)
{
        if (str)
                return sstr_new_len(str, strlen(str));
        else
                return sstr_new_len(str, 0);
}

sstr sstr_empty(void)
{
        return sstr_new_len(NULL, 0);
}

void sstr_free(sstr s)
{
        if (s == NULL) return;
        free(s - sizeof(struct sstr_hdr));
}

size_t sstr_len(const sstr s)
{
        struct sstr_hdr *ss = (void*)(s - sizeof(struct sstr_hdr));
        return ss->len;
}

size_t sstr_avail(const sstr s)
{
        struct sstr_hdr *ss = (void*)(s - sizeof(struct sstr_hdr));
        return ss->free;
}

sstr sstr_make_room_for(sstr s, size_t addlen)
{
        struct sstr_hdr *ss, *newsh;
        size_t free = sstr_avail(s);
        size_t len,newlen;
        
        if (free >= addlen) return s;
        len = sstr_len(s);
        ss = (void*)(s - sizeof(struct sstr_hdr));
        newlen = len + addlen;
        if (newlen < SSTR_MAX_PREALLOC)
                newlen *= 2;
        else
                newlen += SSTR_MAX_PREALLOC;
        newsh = realloc(ss, sizeof(struct sstr_hdr)+newlen+1);
        if (newsh == NULL) return NULL;

        newsh->free = newlen - len;
        return newsh->buf;
}

sstr sstr_catlen(sstr s, const void *t, size_t len)
{
        struct sstr_hdr *ss;
        size_t curlen = sstr_len(s);

        s = sstr_make_room_for(s, len);
        if (s == NULL) return NULL;
        ss = (void*)(s - sizeof(struct sstr_hdr));
        memcpy(s+curlen, t, len);
        ss->len = curlen+len;
        ss->free = ss->free - len;
        s[curlen+len] = '\0';
        return s;
}

sstr sstr_cat(sstr s, const char *t)
{
        return sstr_catlen(s, t, strlen(t));
}

sstr sstr_catvprintf(sstr s, const char *fmt, va_list ap)
{
    va_list cpy;
    char staticbuf[1024], *buf = staticbuf, *t;
    size_t buflen = strlen(fmt)*2;

    /* We try to start using a static buffer for speed.
     * If not possible we revert to heap allocation. */
    if (buflen > sizeof(staticbuf)) {
        buf = malloc(buflen);
        if (buf == NULL) return NULL;
    } else {
        buflen = sizeof(staticbuf);
    }

    /* Try with buffers two times bigger every time we fail to
     * fit the string in the current buffer size. */
    while(1) {
        buf[buflen-2] = '\0';
        va_copy(cpy,ap);
        vsnprintf(buf, buflen, fmt, cpy);
        va_end(cpy);
        if (buf[buflen-2] != '\0') {
            if (buf != staticbuf) free(buf);
            buflen *= 2;
            buf = malloc(buflen);
            if (buf == NULL) return NULL;
            continue;
        }
        break;
    }

    /* Finally concat the obtained string to the SDS string and return it. */
    t = sstr_cat(s, buf);
    if (buf != staticbuf) free(buf);
    return t;
}










#ifdef TEST_SSTR

/**
 * Test cases
 */
void case_sstr_all()
{
        sstr str_nil;
        
        str_nil = sstr_new(NULL);
        assert(str_nil);
        assert(sstr_len(str_nil) == 0);
        sstr_free(str_nil);

        sstr str_str;
        str_str = sstr_new("init_with_str");
        assert(str_str);
        assert(sstr_len(str_str) == strlen("init_with_str"));
        assert(sstr_avail(str_str) == 0);
        assert(!memcmp(str_str, "init_with_str", strlen("init_with_str")));

        str_str = sstr_cat(str_str, " suffix");
        assert(str_str);
        assert(!memcmp(str_str, "init_with_str suffix", strlen("init_with_str suffix")));

        str_str = sstr_catlen(str_str, " a", 2);
        assert(str_str);
        assert(!memcmp(str_str, "init_with_str suffix a", strlen("init_with_str suffix a")));
        sstr_free(str_str);
}

sstr _case_sstr_vcat(sstr s, const char *fmt, ...)
{
        sstr str;
        va_list ap;
        
        va_start(ap, fmt);
        str = sstr_catvprintf(s, fmt, ap);
        va_end(ap);

        return str;
}

void case_sstr_vcat()
{
        sstr str_vcat;

        str_vcat = sstr_new("hello");
        assert(str_vcat);
        
        str_vcat = _test_sstr_vcat(str_vcat, " a%s", "bc");
        assert(!memcmp(str_vcat, "hello abc", strlen("hello abc")));
        sstr_free(str_vcat);
}




int main(int argc, char *argv[])
{
        case_sstr_all();
        case_sstr_vcat();
        

        return 0;
}

#endif
