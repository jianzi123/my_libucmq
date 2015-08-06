#include "ut_public.h"



void case_cmd_decr()
{
        /**
         * decr no_exist key
         * decr string-type key
         * decr character string-type key
         * decr list-type key
         */

        answer_t *ans;
        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);
        ans = kv_ask("flushdb", strlen("flushdb"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);
        
        ans = kv_ask("decr no_exist_key", strlen("decr no_exist_key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "-1");
        answer_release(ans);

        ans = kv_ask("get no_exist_key", strlen("get no_exist_key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "-1");
        answer_release(ans);

        /* string-type key */
        ans = kv_ask("set key 123", strlen("set key 123"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);

        ans = kv_ask("decr key", strlen("decr key"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "122");
        answer_release(ans);

        /** character string-type key */
        ans = kv_ask("set key1 abcd", strlen("set key1 abcd"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);

        ans = kv_ask("decr key1", strlen("decr key1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_VALUE);
        answer_release(ans);

        ans = kv_ask("get key1", strlen("get key1"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "abcd");
        answer_release(ans);
        
        /* list-type key */
        ans = kv_ask("lpush lkey a b c", strlen("lpush lkey a b c"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "3");
        answer_release(ans);

        ans = kv_ask("decr lkey", strlen("decr lkey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_TYPE);
        answer_release(ans);
}


#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE_SHORT(case_cmd_decr);
        UTIL_RUN();
        UTIL_UNINIT(); 

        return 0;
}
#endif
