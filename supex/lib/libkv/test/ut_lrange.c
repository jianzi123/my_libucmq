#include "ut_public.h"




void case_cmd_lrange_without_found()
{
        answer_t *ans;
        answer_iter_t *iter;
        answer_value_t *value;
        

        CU_ASSERT(kv_init(NULL) == ERR_NONE);

        ans = kv_ask("flushdb", strlen("flushdb"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        answer_release(ans);
        
        ans = kv_ask("lrange mylist 0 1", strlen("lrange mylist 0 1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        CU_ASSERT_PTR_EQUAL(answer_first_value(ans), NULL);
        answer_release(ans);

        kv_uninit();
}

void case_cmd_lrange_found()
{
        answer_t *ans;
        answer_iter_t *iter;
        answer_value_t *value;
        

        CU_ASSERT(kv_init(NULL) == ERR_NONE);

        ans = kv_ask("lpush mylist a b c", strlen("lpush mylist a b c"));
        CU_ASSERT(ans->errnum == ERR_NONE);
        answer_release(ans);

        ans = kv_ask("lrange mylist 0 0", strlen("lrange mylist 0 0"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        iter = answer_get_iter(ans, ANSWER_HEAD);
        value = answer_next(iter);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(value), "c");
        CU_ASSERT_PTR_EQUAL(answer_value_to_string(answer_next(iter)), NULL);
        answer_release_iter(iter);
        answer_release(ans);

        ans = kv_ask("lrange mylist 0 2", strlen("lrange mylist 0 2"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        iter = answer_get_iter(ans, ANSWER_HEAD);
        value = answer_next(iter);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(value), "c");
        value = answer_next(iter);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(value), "b");
        value = answer_next(iter);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(value), "a");
        value = answer_next(iter);
        CU_ASSERT_PTR_EQUAL(answer_value_to_string(value), NULL);
        answer_release_iter(iter);
        answer_release(ans);

        ans = kv_ask("lrange mylist 0 3", strlen("lrange mylist 0 3"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        iter = answer_get_iter(ans, ANSWER_HEAD);
        value = answer_next(iter);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(value), "c");
        value = answer_next(iter);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(value), "b");
        value = answer_next(iter);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(value), "a");
        value = answer_next(iter);
        CU_ASSERT_PTR_EQUAL(answer_value_to_string(value), NULL);
        answer_release_iter(iter);
        answer_release(ans);

        ans = kv_ask("lrange mylist 0 -1", strlen("lrange mylist 0 -1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        iter = answer_get_iter(ans, ANSWER_HEAD);
        value = answer_next(iter);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(value), "c");
        value = answer_next(iter);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(value), "b");
        value = answer_next(iter);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(value), "a");
        value = answer_next(iter);
        CU_ASSERT_PTR_EQUAL(answer_value_to_string(value), NULL);
        answer_release_iter(iter);
        answer_release(ans);

        ans = kv_ask("lrange mylist -1 -1", strlen("lrange mylist -1 -1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        iter = answer_get_iter(ans, ANSWER_HEAD);
        value = answer_next(iter);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(value), "a");
        value = answer_next(iter);
        CU_ASSERT_PTR_EQUAL(answer_value_to_string(value), NULL);
        answer_release_iter(iter);
        answer_release(ans);

        ans = kv_ask("lrange mylist -1 1", strlen("lrange mylist -1 1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_OUT_OF_RANGE);
        answer_release(ans);

        ans = kv_ask("lrange mylist 1 -1", strlen("lrange mylist 1 -1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        iter = answer_get_iter(ans, ANSWER_HEAD);
        value = answer_next(iter);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(value), "b");
        value = answer_next(iter);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(value), "a");
        value = answer_next(iter);
        CU_ASSERT_PTR_EQUAL(answer_value_to_string(value), NULL);
        answer_release_iter(iter);
        answer_release(ans);

        kv_uninit();
}

void case_cmd_lrange_uninit()
{
        answer_t *ans;

        kv_uninit();
        ans = kv_ask("lrange lkey 0 -1", strlen("lrange lkey 0 -1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NOT_INIT);
        CU_ASSERT_PTR_EQUAL(answer_first_value(ans), NULL);
        answer_release(ans);
}

void case_cmd_lrange_invalid_param()
{
        answer_t *ans;

        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);
        ans = kv_ask("lrange lkey 0", strlen("lrange lkey 0"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_PTR_EQUAL(answer_first_value(ans), NULL);
        answer_release(ans);
        
        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);
        ans = kv_ask("lrange lkey 0 -1 2", strlen("lrange lkey 0 -1 2"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_PTR_EQUAL(answer_first_value(ans), NULL);
        answer_release(ans);

        kv_uninit();
}

void case_cmd_lrange_mem_leak()
{
        answer_t *ans;
        int i;
        static char buf[1000+128];

        kv_uninit();
        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);
        ans = kv_ask("flushdb", strlen("flushdb"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        answer_release(ans);

        for (i = 1; i <= 1000; i++) {
                ans = kv_ask("lrange lkey -1 -1", strlen("lrange lkey -1 -1"));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
                answer_release(ans);
        }

        kv_uninit();
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}


#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();

        UTIL_ADD_CASE_SHORT(case_cmd_lrange_without_found);
        UTIL_ADD_CASE_SHORT(case_cmd_lrange_found);
        UTIL_ADD_CASE_SHORT(case_cmd_lrange_uninit);
        UTIL_ADD_CASE_SHORT(case_cmd_lrange_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_lrange_mem_leak);
        
        UTIL_RUN();
        UTIL_UNINIT();

        return 0;
}
#endif
