#include "ut_public.h"




void case_cmd_select()
{
        answer_t *ans;
/*
        CU_ASSERT(kv_init(NULL) == ERR_NONE);
        ans = kv_ask("select 0");
        assert(ans->errnum == ERR_NONE);
        answer_print_release(ans);

        ans = kv_ask("set key test_value_from_db0");
        CU_ASSERT(ans->errnum == ERR_NONE);
        answer_print_release(ans);
        
        ans = kv_ask("get key");
        answer_print_release(ans);
        
        ans = kv_ask("select 1");
        CU_ASSERT(ans->errnum == ERR_NONE);
        answer_print_release(ans);

        ans = kv_ask("set key test_value_from_db1");
        CU_ASSERT(ans->errnum == ERR_NONE);
        answer_print_release(ans);
        
        ans = kv_ask("get key");
        answer_print_release(ans);

        ans = kv_ask("select 0");
        answer_print_release(ans);
        
        ans = kv_ask("get key");
        CU_ASSERT(ans->errnum == ERR_NONE);
        answer_print_release(ans);
*/
}



#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE("case_cmd_select():", case_cmd_select);
        UTIL_RUN();
        UTIL_UNINIT();

        return 0;
}
#endif
