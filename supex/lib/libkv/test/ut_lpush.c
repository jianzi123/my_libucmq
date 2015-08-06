#include "ut_public.h"




void case_cmd_lpush_add_few_bytes()
{
        answer_t *ans;
        answer_iter_t *iter;
        answer_value_t *value;

        kv_uninit();
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
        CU_ASSERT(kv_init(NULL) == ERR_NONE);
        ans = kv_ask("lpush lkey a", strlen("lpush lkey a"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "1");
        answer_release(ans);

        ans = kv_ask("lpush lkey b", strlen("lpush lkey b"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "2");
        answer_release(ans);

        ans = kv_ask("lpush lkey c", strlen("lpush lkey c"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "3");
        answer_release(ans);

        ans = kv_ask("lpush lkey d e f g h", strlen("lpush lkey d e f g h"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "8");
        answer_release(ans);

        ans = kv_ask("dbsize", strlen("dbsize"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "1");
        answer_release(ans);

        ans = kv_ask("lrange lkey 0 -1", strlen("lrange lkey 0 -1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);

        iter = answer_get_iter(ans, ANSWER_HEAD);
        value = answer_next(iter);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(value), "h");
        value = answer_next(iter);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(value), "g");
        value = answer_next(iter);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(value), "f");
        value = answer_next(iter);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(value), "e");
        value = answer_next(iter);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(value), "d");
        value = answer_next(iter);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(value), "c");
        value = answer_next(iter);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(value), "b");
        value = answer_next(iter);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(value), "a");
        CU_ASSERT_PTR_EQUAL(answer_next(iter), NULL);

        answer_release_iter(iter);
        answer_release(ans);

        kv_uninit();
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_lpush_add_lot_bytes()
{
        int i;
        answer_t *ans;
        answer_iter_t *iter;
        answer_value_t *value;
        case_data_t *dc[1000];
        static char cmd[10000 + 128];

        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);

        for (i = 1; i <= 1000; i++) {
                dc[i-1] = generator_rand_str(i);
        }

        for (i = 1; i <= 1000; i++) {
                bzero(cmd, sizeof(cmd));
                strcat(cmd, "lpush lkey ");
                memcpy(cmd + strlen("lpush lkey "), dc[i-1]->ptr, dc[i-1]->ptrlen);
                ans = kv_ask(cmd, dc[i-1]->ptrlen + strlen("lpush lkey "));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                answer_release(ans);
        }

        ans = kv_ask("lrange lkey 0 -1", strlen("lrange lkey 0 -1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        iter = answer_get_iter(ans, ANSWER_HEAD);

        for (i = 1000; i >= 1; i--) {
                value = answer_next(iter);
                CU_ASSERT_EQUAL(memcmp(value->ptr, dc[i-1]->ptr, dc[i-1]->ptrlen), 0);
        }
        
        answer_release_iter(iter);
        answer_release(ans);

        for (i = 1; i <= 1000; i++) {
                case_data_release(dc[i-1]);
        }
        
        kv_uninit();
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_lpush_del()
{
        answer_t *ans;


        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);
        ans = kv_ask("lpush lkey 123 456", strlen("lpush lkey 123 456"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        answer_release(ans);
        
        ans = kv_ask("del lkey", strlen("del lkey"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "1");
        answer_release(ans);

        ans = kv_ask("lrange lkey 0 -1", strlen("lrange lkey 0 -1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        answer_release(ans);
}

void case_cmd_lpush_invalid_param()
{
        answer_t *ans;
        
        kv_uninit();
        CU_ASSERT(kv_init(NULL) == ERR_NONE);

        ans = kv_ask("lpush", strlen("lpush"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        answer_release(ans);

        ans = kv_ask("lpush lkey", strlen("lpush lkey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        answer_release(ans);
}



#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE_SHORT(case_cmd_lpush_add_few_bytes);
        UTIL_ADD_CASE_SHORT(case_cmd_lpush_add_lot_bytes);
        UTIL_ADD_CASE_SHORT(case_cmd_lpush_del);
        UTIL_ADD_CASE_SHORT(case_cmd_lpush_invalid_param);
        UTIL_RUN();
        UTIL_UNINIT();

        return 0;
}
#endif
