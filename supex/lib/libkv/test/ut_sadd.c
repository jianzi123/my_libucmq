#include "ut_public.h"
#include <stdlib.h>



void cmd_case_sadd_normal()
{
        int i;
        answer_t *ans;
        answer_iter_t *iter;
        answer_value_t *value;
        char short_buf[32];
        static char long_buf[1024*1024];
        
        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);
        
        ans = kv_ask("sadd skey a b c d e", strlen("sadd skey a b c d e"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "5");
        answer_release(ans);

        ans = kv_ask("smembers skey", strlen("smembers skey"));
        CU_ASSERT_EQUAL(answer_length(ans), 5);
        iter = answer_get_iter(ans, ANSWER_HEAD);
        while((value = answer_next(iter)) != NULL) {
                CU_ASSERT(!strcmp(answer_value_to_string(value), "a") ||
                                !strcmp(answer_value_to_string(value), "b") ||
                                !strcmp(answer_value_to_string(value), "c") ||
                                !strcmp(answer_value_to_string(value), "d") ||
                                !strcmp(answer_value_to_string(value), "e")
                        );
        }
        answer_release_iter(iter);
        answer_release(ans);

        
        ans = kv_ask("flushdb", strlen("flushdb"));
        answer_release(ans);

        strcpy(long_buf, "sadd skey ");
        for (i = 1; i <= 1024; i++) {
                snprintf(short_buf, sizeof(short_buf), "%d ", i);
                strcat(long_buf, short_buf);
        }

        ans = kv_ask(long_buf, strlen(long_buf));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "1024");
        answer_release(ans);

        ans = kv_ask("smembers skey", strlen("smembers skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        answer_release(ans);

        ans = kv_ask("del skey", strlen("del skey"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "1");
        answer_release(ans);

        ans = kv_ask("sadd skey a b c", strlen("sadd skey a b c"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "3");
        answer_release(ans);

        ans = kv_ask("sadd skey b c", strlen("sadd skey b c"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "0");
        answer_release(ans);

        ans = kv_ask("sadd skey b c d", strlen("sadd skey b c d"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "1");
        answer_release(ans);
        
        kv_uninit();
}

void cmd_case_sadd_invalid_cmd()
{
        kv_init(NULL);
        
        answer_t *ans;
        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);
        answer_release(kv_ask("flushdb", strlen("flushdb")));

        ans = kv_ask("saddx", strlen("saddx"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
        answer_release(ans);
        
        kv_uninit();
}

void cmd_case_sadd_invalid_param()
{
        answer_t *ans;
        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);

        ans = kv_ask("sadd", strlen("sadd"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        answer_release(ans);

        ans = kv_ask("sadd skey", strlen("sadd skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        answer_release(ans);
        
        kv_uninit();
}

void cmd_case_sadd_invalid_type()
{
        answer_t *ans;
        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);

        ans = kv_ask("flushdb", strlen("flushdb"));
        answer_release(ans);

        ans = kv_ask("set key value", strlen("set key value"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        answer_release(ans);
        
        ans = kv_ask("sadd key a b c", strlen("sadd key a b c"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_TYPE);
        answer_release(ans);

        kv_uninit();
}

void cmd_case_sadd_stress()
{
        int i;
        answer_t *ans;
        char short_buf[32];
        static char long_buf[10000*10001/2 + 10];

        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);
        ans = kv_ask("flushdb", strlen("flushdb"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        answer_release(ans);

        strcpy(long_buf, "sadd skey ");
        for (i = 1; i <= 10000; i++) {
                snprintf(short_buf, sizeof(short_buf), "%d ", i);
                strcat(long_buf, short_buf);
        }

        ans = kv_ask(long_buf, strlen(long_buf));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "10000");
        answer_release(ans);
        kv_uninit();
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}


#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE_SHORT(cmd_case_sadd_normal);
        UTIL_ADD_CASE_SHORT(cmd_case_sadd_invalid_cmd);
        UTIL_ADD_CASE_SHORT(cmd_case_sadd_invalid_param);
        UTIL_ADD_CASE_SHORT(cmd_case_sadd_invalid_type);
        UTIL_ADD_CASE_SHORT(cmd_case_sadd_stress);
        UTIL_RUN();
        UTIL_UNINIT();

        return 0;
}
#endif
