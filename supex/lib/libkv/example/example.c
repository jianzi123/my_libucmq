/**
 * Copyright (c) 2015
 * All Rights Reserved
 *
 * @file    example.h
 * @detail  An example to show how to use libkv.
 *          
 * @author  shishengjie
 * @version 0.1
 * @date    2015-05-18
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "libkv.h"



/**
 * @notice value->ptr is probable an bytes array that hold some '\0' characters in any
 *         position, therefore you should handle value->ptr around 0~value->ptrlen
 */
void example(const char *cmd, unsigned int cmdlen)
{
        int i;
        answer_t *ans = NULL;
        answer_iter_t *iter;
        answer_value_t *value;

        if (!cmd || cmdlen == 0) {
                fprintf(stderr, "Invalid command, try again!");
                return;
        }

        ans = kv_ask(cmd, cmdlen);
        if (ans->errnum != ERR_NONE) {
                printf("errnum:%d\terr:%s\n", ans->errnum, ans->err);
                return;
        }
        
        fprintf(stdout, "===>[%s]\n", cmd);
        /** treat as first value to string */
        value = answer_first_value(ans);
        printf("\tOutput first value:%s\n\n", answer_value_to_string(value));

        /** treat as single value */
        iter = answer_get_iter(ans, ANSWER_HEAD);
        while((value = answer_next(iter)) != NULL) {
                printf("\tOutput single value ===> ");
                for (i = 0; i < value->ptrlen; i++) {
                        printf("%c", ((char*)value->ptr)[i]);
                }
                puts("\n");
                break; /** just print first value */
        }

        /** treat as multiple values */
        printf("\tOutput multiple values:\n");
        answer_rewind_iter(ans, iter);
        while((value = answer_next(iter)) != NULL) {
                printf("\t\tvalue content ===> ");
                for (i = 0; i < value->ptrlen; i++) {
                        printf("%c", ((char*)value->ptr)[i]);
                }
                puts("\n");
        }

        answer_release_iter(iter);
        answer_release(ans);
}



int main(int argc, char *argv[])
{
        assert(kv_init(NULL) == ERR_NONE);

        example("lpush lkey a b c", strlen("lpush lkey a b c"));
        example("set key value", strlen("set key value"));
        example("get key", strlen("get key"));
        example("flushdb", strlen("flushdb"));
        example("dbsize", strlen("dbsize"));
        kv_uninit();

        return 0;
}
