#include "ut_public.h"



void case_cmd_flushdb()
{
        /*
         * flush empty db
         * flush one key db
         * flush 5000 string keys and 5000 lpush keys
         */
        
        answer_t *ans;
        static char buf[10000+128];
        int i;
        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);
        ans = kv_ask("flushdb", strlen("flushdb"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);
        
        ans = kv_ask("dbsize", strlen("dbsize"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "0");
        answer_release(ans);

        ans = kv_ask("flushdb", strlen("flushdb"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);
        
        ans = kv_ask("dbsize", strlen("dbsize"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "0");
        answer_release(ans);

        /* flush one key */
        ans = kv_ask("set key value", strlen("set key value"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);

        ans = kv_ask("flushdb", strlen("flushdb"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);
        
        ans = kv_ask("dbsize", strlen("dbsize"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "0");
        answer_release(ans);

        /* flush list key */
        ans = kv_ask("lpush lkey a b c", strlen("lpush lkey a b c"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "3");
        answer_release(ans);

        ans = kv_ask("flushdb", strlen("flushdb"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);
        
        ans = kv_ask("dbsize", strlen("dbsize"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "0");
        answer_release(ans);

        /* 5000 string keys and 5000 list keys */
        for (i = 1; i <= 5000; i++) {
                int bytes = snprintf(buf, sizeof(buf), "set key%d %d", i, i);
                
                ans = kv_ask(buf, bytes);
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
                answer_release(ans);

                bytes = snprintf(buf, sizeof(buf), "lpush lkey%d %d", i, i);
                ans = kv_ask(buf, bytes);
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "1");
                answer_release(ans);
        }

        ans = kv_ask("dbsize", strlen("dbsize"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "10000");
        answer_release(ans);

        ans = kv_ask("flushdb", strlen("flushdb"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);

        ans = kv_ask("dbsize", strlen("dbsize"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "0");
        answer_release(ans);
}





#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        
        UTIL_ADD_CASE_SHORT(case_cmd_flushdb); 
        UTIL_RUN();
        UTIL_UNINIT(); 

        return 0;
}
#endif
