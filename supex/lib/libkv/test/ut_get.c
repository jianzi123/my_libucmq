#include "ut_public.h"





void case_cmd_get()
{
        answer_t *ans;

        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);

        ans = kv_ask("flushdb", strlen("flushdb"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);
        
        ans = kv_ask("get key", strlen("get key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        answer_release(ans);

        ans = kv_ask("set key value", strlen("set key value"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        answer_release(ans);

        ans = kv_ask("get key", strlen("get key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "value");
        answer_release(ans);

        ans = kv_ask("get key key_more", strlen("get key key_more"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        answer_release(ans);

        ans = kv_ask("lpush lkey a b c", strlen("lpush lkey a b c"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "3");
        answer_release(ans);

        ans = kv_ask("get lkey", strlen("get lkey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_TYPE);
        answer_release(ans);
}














#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE_SHORT(case_cmd_get);
        UTIL_RUN();
        UTIL_UNINIT(); 

        return 0;
}
#endif
