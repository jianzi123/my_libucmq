#include "ut_public.h"



void case_cmd_pexpire_get()
{
        answer_t *ans;
        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);

        ans = kv_ask("flushdb", strlen("flushdb"));
        answer_release(ans);

        ans = kv_ask("set key value", strlen("set key value"));
        answer_release(ans);

        ans = kv_ask("pexpire key 5000", strlen("pexpire key 5000"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "1");
        answer_release(ans);

        sleep(3);
        ans = kv_ask("get key", strlen("get key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "value");
        answer_release(ans);

        sleep(3);

        ans = kv_ask("get key", strlen("get key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        CU_ASSERT_PTR_EQUAL(answer_value_to_string(answer_first_value(ans)), NULL);
        answer_release(ans);

        ans = kv_ask("dbsize", strlen("dbsize"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "0");
        answer_release(ans);

        
        ans = kv_ask("set key value", strlen("set key value"));
        answer_release(ans);
        
        ans = kv_ask("pexpire key 2000", strlen("pexpire key 2000"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "1");
        answer_release(ans);
        sleep(3);

        ans = kv_ask("del key", strlen("del key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "0");
        answer_release(ans);

        answer_release(kv_ask("set key value", strlen("set key value")));
        
        ans = kv_ask("del key", strlen("del key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "1");
        answer_release(ans);

        kv_uninit();
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_pexpire_list()
{
        answer_t *ans;
        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);
        ans = kv_ask("flushdb", strlen("flushdb"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);

        ans = kv_ask("lpush lkey a b c", strlen("lpush lkey a b c"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "3");
        answer_release(ans);

        ans = kv_ask("pexpire lkey 1000", strlen("pexpire lkey 1000"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "1");
        answer_release(ans);

        ans = kv_ask("lrange lkey 0 -1", strlen("lrange lkey 0 -1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        answer_release(ans);

        sleep(2);
        ans = kv_ask("lrange lkey 0 -1", strlen("lrange lkey 0 -1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        answer_release(ans);

        kv_uninit();
}

void case_cmd_pexpire_set()
{
        answer_t *ans;
        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);
        ans = kv_ask("flushdb", strlen("flushdb"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);

        ans = kv_ask("sadd skey a b c", strlen("sadd skey a b c"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "3");
        answer_release(ans);

        ans = kv_ask("pexpire skey 1000", strlen("pexpire skey 1000"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "1");
        answer_release(ans);

        ans = kv_ask("smembers skey", strlen("smembers skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        answer_release(ans);

        sleep(2);
        ans = kv_ask("smembers skey", strlen("smembers skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        answer_release(ans);

        kv_uninit();
}

void case_cmd_pexpire_without_key()
{
        answer_t *ans;
        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);
        ans = kv_ask("flushdb", strlen("flushdb"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);

        ans = kv_ask("pexpire key 10000", strlen("pexpire key 10000"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "0");
        answer_release(ans);
        
        kv_uninit();
}

void case_cmd_pexpire_invalid_cmd()
{
        answer_t *ans;
        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);
        ans = kv_ask("expir key 10000", strlen("expir key 10000"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
        CU_ASSERT_PTR_EQUAL(answer_value_to_string(answer_first_value(ans)), NULL);
        answer_release(ans);
        
        kv_uninit();
}

void case_cmd_pexpire_invalid_param()
{
        answer_t *ans;
        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);

        ans = kv_ask("pexpire", strlen("pexpire"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_PTR_EQUAL(answer_value_to_string(answer_first_value(ans)), NULL);
        answer_release(ans); 
        
        ans = kv_ask("pexpire key", strlen("pexpire key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_PTR_EQUAL(answer_value_to_string(answer_first_value(ans)), NULL);
        answer_release(ans);

        ans = kv_ask("pexpire key 10000 20000", strlen("pexpire key 10000 20000"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_PTR_EQUAL(answer_value_to_string(answer_first_value(ans)), NULL);
        answer_release(ans);

        ans = kv_ask("pexpire key 123456789012345678901234567890", strlen("pexpire key 123456789012345678901234567890"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_VALUE);
        CU_ASSERT_PTR_EQUAL(answer_value_to_string(answer_first_value(ans)), NULL);
        answer_release(ans);
        
        kv_uninit();
}

void case_cmd_pexpire_stress()
{
        int i;
        answer_t *ans;
        static char buf[10000+32];

        kv_uninit();
        kv_init(NULL);
        ans = kv_ask("set key value", strlen("set key value"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        answer_release(ans);
        
        for (i = 10000; i >= 1; i--) {
                snprintf(buf, sizeof(buf), "pexpire key %d", i);
                ans = kv_ask(buf, strlen(buf));
                CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "1");
                answer_release(ans);
        }

        ans = kv_ask("get key", strlen("get key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "value");
        answer_release(ans);

        sleep(2);
        ans = kv_ask("get key", strlen("get key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        CU_ASSERT_PTR_EQUAL(answer_value_to_string(answer_first_value(ans)), NULL);
        answer_release(ans);
        
        kv_uninit();
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}



#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE_SHORT(case_cmd_pexpire_get);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpire_list);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpire_set);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpire_without_key);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpire_invalid_cmd);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpire_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpire_stress);
        UTIL_RUN();
        UTIL_UNINIT(); 

        return 0;
}
#endif
