#include "ut_public.h"



void case_cmd_decrby()
{
        answer_t *ans;
        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);
        ans = kv_ask("flushdb", strlen("flushdb"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);
        /** no exist key */
        ans = kv_ask("decrby no_exist_key 123", strlen("decrby no_exist_key 123"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "-123");
        answer_release(ans);

        ans = kv_ask("get no_exist_key", strlen("get no_exist_key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "-123");
        answer_release(ans);
        
        /** exist key */
        ans = kv_ask("set key 123", strlen("set key 123"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);

        ans = kv_ask("decrby key 12345678901234567890123456789012345678901234567890", strlen("decrby key 12345678901234567890123456789012345678901234567890"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_VALUE);
        answer_release(ans);
        
        ans = kv_ask("decrby key abcd", strlen("decrby key abcd"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_VALUE);
        answer_release(ans);
        
        ans = kv_ask("decrby key 124", strlen("decrby key 124"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "-1");
        answer_release(ans);

        ans = kv_ask("get key", strlen("get key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "-1");
        answer_release(ans);

        /** invalid number */
        ans = kv_ask("decrby no_exist_key1 abcd", strlen("decrby no_exist_key1 abcd"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_VALUE);
        answer_release(ans);

        /** invalid value */
        ans = kv_ask("set key2 abcd", strlen("set key2 abcd"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);
        
        ans = kv_ask("decrby key2 123", strlen("decrby key2 123"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_VALUE);
        answer_release(ans);

        /** list */
        ans = kv_ask("lpush lkey a b c", strlen("lpush lkey a b c"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "3");
        answer_release(ans);

        ans = kv_ask("decrby lkey 456", strlen("decrby lkey 456"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_TYPE);
        answer_release(ans);
}


#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE_SHORT(case_cmd_decrby);
        UTIL_RUN();
        UTIL_UNINIT(); 

        return 0;
}
#endif
