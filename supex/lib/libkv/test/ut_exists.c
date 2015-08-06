#include "ut_public.h"



void case_cmd_exists_normal()
{
        answer_t *ans;
        
        kv_uninit();
        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);

        ans = kv_ask("set key value", strlen("set key value"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);

        ans = kv_ask("exists key", strlen("exists key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "1");
        answer_release(ans);

        ans = kv_ask("exists key_noexist", strlen("exists key_noexist"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "0");
        answer_release(ans);

        ans = kv_ask("del key", strlen("del key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "1");
        answer_release(ans);
        
        ans = kv_ask("exists key", strlen("exists key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "0");
        answer_release(ans);

        kv_uninit();
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_exists_invalid_cmd()
{
        answer_t *ans;

        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);

        ans = kv_ask("exist key", strlen("exist key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
        CU_ASSERT_PTR_EQUAL(answer_value_to_string(answer_first_value(ans)), NULL);
        answer_release(ans);
        kv_uninit();
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_exists_invalid_param()
{
        answer_t *ans;

        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);

        ans = kv_ask("exists", strlen("exists"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_PTR_EQUAL(answer_value_to_string(answer_first_value(ans)), NULL);
        answer_release(ans);

        ans = kv_ask("exists invalid1 invalid2", strlen("exists invalid1 invalid2"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_PTR_EQUAL(answer_value_to_string(answer_first_value(ans)), NULL);
        answer_release(ans);
        kv_uninit();
}


#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE_SHORT(case_cmd_exists_normal);
        UTIL_ADD_CASE_SHORT(case_cmd_exists_invalid_cmd);
        UTIL_ADD_CASE_SHORT(case_cmd_exists_invalid_param);
        UTIL_RUN();
        UTIL_UNINIT();

        return 0;
}
#endif
