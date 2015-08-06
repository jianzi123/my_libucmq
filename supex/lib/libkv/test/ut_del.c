#include "ut_public.h"





void case_cmd_del()
{
        /*
         * delete no_exist key
         * delete a exist key
         * delete multiple exist keys
         * delete multiple keys with some no_exist
         */
        answer_t *ans;

        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);

        ans = kv_ask("flushdb", strlen("flushdb"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);

        ans = kv_ask("del key_noexist", strlen("del key_noexist"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "0");
        answer_release(ans);
        
        ans = kv_ask("set key value", strlen("set key value"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);
        
        ans = kv_ask("del key", strlen("del key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "1");
        answer_release(ans);

        ans = kv_ask("set key1 value1", strlen("set key1 value1"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);
        ans = kv_ask("set key2 value2", strlen("set key2 value2"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);
        ans = kv_ask("set key3 value3", strlen("set key3 value3"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);

        ans = kv_ask("del key1 key2 key3", strlen("del key1 key2 key3"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "3");
        answer_release(ans);

        ans = kv_ask("dbsize", strlen("dbsize"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "0");
        answer_release(ans);

        ans = kv_ask("set key4 value4", strlen("set key4 value4"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);
        ans = kv_ask("set key5 value5", strlen("set key5 value5"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);
        ans = kv_ask("set key6 value6", strlen("set key6 value6"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);

        ans = kv_ask("del key4 key5 key7", strlen("del key4 key5 key7"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "2");
        answer_release(ans);

        ans = kv_ask("dbsize", strlen("dbsize"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "1");
        answer_release(ans);
}




#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE_SHORT(case_cmd_del);
        UTIL_RUN();
        UTIL_UNINIT(); 

        return 0;
}
#endif
