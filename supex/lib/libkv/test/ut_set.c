#include "ut_public.h"



void case_cmd_set_normal()
{
        char cmd[1024];
        answer_t *ans;
        answer_iter_t *iter;
        answer_value_t *value;
        
        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);
        ans = kv_ask("set key value", strlen("set key value"));
        CU_ASSERT_STRING_EQUAL("OK", answer_value_to_string(answer_first_value(ans)));
        answer_release(ans);

        ans = kv_ask("set key value", strlen("set key value"));
        CU_ASSERT_STRING_EQUAL("OK", answer_value_to_string(answer_first_value(ans)));
        answer_release(ans);

        ans = kv_ask("set key value_new", strlen("set key value_new"));
        CU_ASSERT_STRING_EQUAL("OK", answer_value_to_string(answer_first_value(ans)));
        answer_release(ans);

        ans = kv_ask("set key_new value", strlen("set key_new value"));
        CU_ASSERT_STRING_EQUAL("OK", answer_value_to_string(answer_first_value(ans)));
        answer_release(ans);

        ans = kv_ask("set key_new value", strlen("set key_new value"));
        CU_ASSERT_STRING_EQUAL("OK", answer_value_to_string(answer_first_value(ans)));
        answer_release(ans);

        ans = kv_ask("set key_new value", strlen("set key_new value"));
        CU_ASSERT_STRING_EQUAL("OK", answer_value_to_string(answer_first_value(ans)));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
        answer_release(ans);
        
        kv_uninit();
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_set_invalid_param()
{
        answer_t *ans;
        
        kv_uninit();
        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);
        
        ans = kv_ask("set", strlen("set"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_PTR_EQUAL(ans->head, NULL);
        CU_ASSERT_PTR_EQUAL(ans->tail, NULL);
        answer_release(ans);

        ans = kv_ask("set key", strlen("set key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_PTR_EQUAL(ans->head, NULL);
        CU_ASSERT_PTR_EQUAL(ans->tail, NULL);
        answer_release(ans);

        ans = kv_ask("set key value", strlen("set key value"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        answer_release(ans);

        ans = kv_ask("set key value1 more", strlen("set key value1 more"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_PTR_EQUAL(ans->head, NULL);
        CU_ASSERT_PTR_EQUAL(ans->tail, NULL);
        answer_release(ans);
}

void case_cmd_set_invalid_type()
{
        answer_t *ans;
        
        kv_uninit();
        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);
        ans = kv_ask("lpush lkey a b c", strlen("lpush lkey a b c"));
        CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "3");
        answer_release(ans);

        ans = kv_ask("set lkey lvalue", strlen("set lkey lvalue"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        answer_release(ans);

        ans = kv_ask("lpush lkey d e", strlen("lpush lkey d e"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_TYPE);
        answer_release(ans);
        
}

void case_cmd_set_stress()
{
        /* key1 ~ key10000, value -> random */
        /* set */
        answer_t *ans;
        answer_iter_t *iter;
        answer_value_t *value;
        int i;
        static char cmd[10000+1];
        case_data_t *cd[10000];

        CU_ASSERT_EQUAL(kv_init(NULL), ERR_NONE);
        for (i = 1; i <= 10000 ; i++) {
                int bytes = snprintf(cmd, sizeof(cmd), "set key%d ", i);
                cd[i-1] = generator_rand_str(i);
                memcpy(cmd + bytes, cd[i-1]->ptr, cd[i-1]->ptrlen);
                
                ans = kv_ask(cmd, bytes + cd[i-1]->ptrlen);
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(answer_value_to_string(answer_first_value(ans)), "OK");
                answer_release(ans);
        }

        for (i = 1; i <= 10000; i++) {
                case_data_release(cd[i-1]);
        }
}




#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE_SHORT(case_cmd_set_normal);
        UTIL_ADD_CASE_SHORT(case_cmd_set_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_set_invalid_type);
        UTIL_ADD_CASE_SHORT(case_cmd_set_stress);
        UTIL_RUN();
        UTIL_UNINIT(); 

        return 0;
}
#endif
