/*
 * Copyright:   None
 * File Name:   ut_case_data.c
 * Author:      shishengjie@daoke.me
 * Date:        2015/05/12
 * Descrition:  Test case data generator.
 * History:     None
 */

#include "ut_data_generator.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>


void zfree(void* ptr);
void* zmalloc(size_t size);


static char* get_random_bytes_from_dev(unsigned int len)
{
        char *buf;
        int fd;
        int readbytes;
        if (len == 0) return NULL;

        buf = zmalloc(len); /** append '\0' */
        
        fd = open("/dev/urandom", O_RDONLY);
        readbytes = read(fd, buf, len);
        assert(readbytes == len);
        close(fd);

        return buf;
}

void case_data_release(case_data_t *dc)
{
        if (dc == NULL) return;
        if (dc->ptr != NULL) zfree((void*)dc->ptr);
        zfree((void*)dc);
}

case_data_t* case_data_create(const char* ptr, unsigned int ptrlen)
{
        case_data_t *dc = zmalloc(sizeof(*dc));
        dc->ptr = ptr;
        dc->ptrlen = ptrlen;
        return dc;
}

case_data_t* generator_rand_str(unsigned int len)
{
        int i = 0;
        char *buf;
        char *ready;
        case_data_t *dc;

        if (len == 0) return NULL;
        dc = zmalloc(sizeof(*dc));
        ready = zmalloc(len + 1);
        
        while(i < len) {
                buf = get_random_bytes_from_dev(1);
                buf[0] = fabs(buf[0]%127);
                if (buf[0] < 33 || buf[0] > 126) {
                        zfree(buf);
                        continue;
                } else {
                        ready[i++] = buf[0];
                        zfree(buf);
                }
        }

        ready[len] = '\0';

        dc->ptr = ready;
        dc->ptrlen = len;
        return dc;
}

case_data_t* generator_rand_bytes(unsigned int len)
{
        char *buf;

        buf = get_random_bytes_from_dev(len);
        if (buf == NULL) return NULL;
        
        case_data_t *dc = zmalloc(sizeof(*dc));
        dc->ptr = buf;
        dc->ptrlen = len;
        return dc;
}

unsigned int gen_rand_num(unsigned int max)
{
        char *buf;
        unsigned long sum = 0;

        buf = get_random_bytes_from_dev(4);
        
        sum = buf[0]^buf[1]^buf[2]^buf[3];
        zfree(buf);
        return fabs(sum % max);
}
