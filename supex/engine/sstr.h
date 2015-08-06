/**
 * @file   sstr.h
 * @brief  Structured string declarations.(refer to sds implementation of redis)
 *
 * @author shishengjie
 * @date   2015-06-15
 */


#ifndef STR_H_
#define STR_H_


#define SSTR_MAX_PREALLOC (1024*1024)


#include <sys/types.h>
#include <stdarg.h>

typedef char *sstr;

struct sstr_hdr {
        size_t len;
        size_t free;
        char buf[];
};


sstr sstr_new(const char *str);
sstr sstr_new_len(const void *init, size_t len);
sstr sstr_empty(void);
void sstr_free(sstr s);
size_t sstr_len(const sstr s);
size_t sstr_avail(const sstr s);
sstr sstr_catlen(sstr s, const void *t, size_t len);
sstr sstr_cat(sstr s, const char *t);
sstr sstr_catvprintf(sstr s, const char *fmt, va_list ap);







#endif
