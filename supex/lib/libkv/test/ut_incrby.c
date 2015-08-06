#include "ut_public.h"



void case_cmd_incrby()
{
        /**
         * incr valid number to no_exist key
         * incr valid number to exist key(string-type)
         * incr valid number to exist key(lpush-type)
         * incr invalid number to no_exist key
         * incr invalid number to exist key(string-type)
         * incr invalid number to exist key(lpush-type)
         */

        answer_t *ans;
        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);
        ans = kv_ask("flushdb", strlen("flushdb"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);
        /** no exist key */
        ans = kv_ask("incrby no_exist_key 123", strlen("incrby no_exist_key 123"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "123");
        answer_release(ans);

        ans = kv_ask("get no_exist_key", strlen("get no_exist_key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "123");
        answer_release(ans);
        
        /** exist key */
        ans = kv_ask("set key 123", strlen("set key 123"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);

        ans = kv_ask("incrby key 123", strlen("incrby key 123"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "246");
        answer_release(ans);

        ans = kv_ask("get key", strlen("get key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "246");
        answer_release(ans);

        /** list */
        ans = kv_ask("lpush lkey a b c", strlen("lpush lkey a b c"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "3");
        answer_release(ans);

        ans = kv_ask("incrby lkey 456", strlen("incr lkey 456"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_TYPE);
        answer_release(ans);

        ans = kv_ask("flushdb", strlen("flushdb"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);

        /**
         * incrby invalid number 
         */
        /** no exist key */
        ans = kv_ask("incrby no_exist_key abcd", strlen("incrby no_exist_key abcd"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_VALUE);
        answer_release(ans);

        /** exist key */
        ans = kv_ask("set key 123", strlen("set key 123"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);

        ans = kv_ask("incrby key abcd", strlen("incr key abcd"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_VALUE);
        answer_release(ans);

        ans = kv_ask("get key", strlen("get key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "123");
        answer_release(ans);

        /** invalid value */
        ans = kv_ask("set key1 abcd", strlen("set key1 abcd"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);
        
        ans = kv_ask("incrby key1 123", strlen("incrby key1 123"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_VALUE);
        answer_release(ans);
        
        /** list */
        ans = kv_ask("lpush lkey a b c", strlen("lpush lkey a b c"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "3");
        answer_release(ans);

        ans = kv_ask("incrby lkey 456", strlen("incr lkey 456"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_TYPE);
        answer_release(ans);

        ans = kv_ask("flushdb", strlen("flushdb"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);
}


#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE_SHORT(case_cmd_incrby);
        UTIL_RUN();
        UTIL_UNINIT(); 

        return 0;
}
#endif
