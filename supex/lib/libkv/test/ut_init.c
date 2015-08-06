#include "ut_public.h"



void case_init()
{
        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);
        kv_uninit();
}

void case_init_multiple()
{
        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);
        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);
        CU_ASSERT_EQUAL(kv_init("param1"), ERR_NONE);
        CU_ASSERT_EQUAL(kv_init("param1", NULL), ERR_NONE);
        CU_ASSERT_EQUAL(kv_init("param1", "value1"), ERR_NONE);
        CU_ASSERT_EQUAL(kv_init("param1", "value1", "param2"), ERR_NONE);
        CU_ASSERT_EQUAL(kv_init("param1", "value1", "param2", "value2"), ERR_NONE);
}

void case_init_without_used()
{
        answer_t *ans;
        answer_iter_t *iter;
        const char* cmd = "set init_test_key init_test_value";
        
        /* without kv_init */
        kv_uninit();
        ans = kv_ask(cmd, strlen(cmd));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NOT_INIT);
        CU_ASSERT_STRING_EQUAL(ans->err, "libkv not init");

        CU_ASSERT_PTR_EQUAL(answer_first_value(ans), NULL);
        CU_ASSERT_PTR_EQUAL(answer_value_to_string(answer_first_value(ans)), NULL);
        
        iter = answer_get_iter(ans, ANSWER_HEAD);
        CU_ASSERT_PTR_EQUAL(answer_next(iter), NULL);
        answer_release_iter(iter);
        answer_release(ans);
}

void case_init_after_used()
{
        answer_t *ans;
        answer_iter_t *iter;
        answer_value_t *value;
        const char* cmd = "set init_test_key init_test_value";

        kv_uninit();
        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);

        ans = kv_ask(cmd, strlen(cmd));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(ans->err, "no error");
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);

        kv_uninit();
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_used_without_init()
{
        answer_t *ans;
        const char* cmd = "set init_test_key init_test_value";
        kv_uninit();

        ans = kv_ask(cmd, strlen(cmd));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NOT_INIT);
        answer_release(ans);
}

void case_init_stress()
{
        int i;

        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);
        kv_uninit();
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);

        for (i = 1; i <= 100000; i++)
                CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);

        for (i = 1; i <= 100000; i++)
                kv_uninit();
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
        
        for (i = 1; i <= 100000; i++) {
                CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);
                kv_uninit();
        }

        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}


#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE_SHORT(case_init);
        UTIL_ADD_CASE_SHORT(case_init_multiple);
        UTIL_ADD_CASE_SHORT(case_init_without_used);
        UTIL_ADD_CASE_SHORT(case_init_after_used);
        UTIL_ADD_CASE_SHORT(case_used_without_init);
        UTIL_ADD_CASE_SHORT(case_init_stress);
        UTIL_RUN();
        UTIL_UNINIT(); 

        return 0;
}
#endif
