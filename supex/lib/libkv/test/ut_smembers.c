#include "ut_public.h"




void cmd_case_smembers_without_members()
{
        answer_t *ans;
        
        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);

        ans = kv_ask("flushdb", strlen("flushdb"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);

        ans = kv_ask("smembers skey", strlen("smembers skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        CU_ASSERT_PTR_EQUAL(answer_first_value(ans), NULL);
        answer_release(ans);

        kv_uninit();
}

void cmd_case_smembers_with_members()
{
        answer_t *ans;
        answer_iter_t *iter;
        answer_value_t *value;
        
        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);

        ans = kv_ask("flushdb", strlen("flushdb"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);

        ans = kv_ask("sadd skey a b c", strlen("sadd skey a b c"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "3");
        answer_release(ans);

        ans = kv_ask("smembers skey", strlen("smembers skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(answer_length(ans), 3);
        iter = answer_get_iter(ans, ANSWER_HEAD);
        while((value = answer_next(iter)) != NULL) {
                CU_ASSERT(!strcmp(answer_value_to_string(value), "a") ||
                          !strcmp(answer_value_to_string(value), "b") ||
                          !strcmp(answer_value_to_string(value), "c"));
        }
        answer_release_iter(iter);
        answer_release(ans);

        kv_uninit();
}

void cmd_case_smembers_invalid_cmd()
{
        answer_t *ans;
        answer_iter_t *iter;
        
        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);

        ans = kv_ask("smemberss skey", strlen("smemberss skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
        CU_ASSERT_PTR_EQUAL(answer_first_value(ans), NULL);
        answer_release(ans);

        kv_uninit();
}

void cmd_case_smembers_invalid_param()
{
        answer_t *ans;
        answer_iter_t *iter;

        kv_uninit();
        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);

        ans = kv_ask("smembers", strlen("smembers"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_PTR_EQUAL(answer_first_value(ans), NULL);
        answer_release(ans);

        kv_uninit(); 
}

void cmd_case_smembers_invalid_type()
{
        answer_t *ans;
        answer_iter_t *iter;

        kv_uninit();
        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);

        ans = kv_ask("set key value", strlen("set key value"));
        answer_release(ans);
        
        ans = kv_ask("smembers key", strlen("smembers key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_TYPE);
        CU_ASSERT_PTR_EQUAL(answer_first_value(ans), NULL);
        answer_release(ans);
        
        kv_uninit(); 
}

void cmd_case_smembers_stress()
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

        ans = kv_ask("smembers skey", strlen("smembers skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(answer_length(ans), 10000);
        answer_release(ans);

        kv_uninit();
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}



#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE_SHORT(cmd_case_smembers_without_members);
        UTIL_ADD_CASE_SHORT(cmd_case_smembers_with_members);
        UTIL_ADD_CASE_SHORT(cmd_case_smembers_invalid_cmd);
        UTIL_ADD_CASE_SHORT(cmd_case_smembers_invalid_param);
        UTIL_ADD_CASE_SHORT(cmd_case_smembers_invalid_type);
        UTIL_ADD_CASE_SHORT(cmd_case_smembers_stress);
        UTIL_RUN();
        UTIL_UNINIT();

        return 0;
}
#endif
