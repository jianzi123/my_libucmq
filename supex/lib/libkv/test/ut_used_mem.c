#include "ut_public.h"




void case_used_memory()
{
        /** get used memory before kv_init */
        kv_uninit();
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
        kv_uninit();
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}







#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE("case_used_memory()", case_used_memory);
        UTIL_RUN();
        UTIL_UNINIT();

        return 0;
}
#endif
