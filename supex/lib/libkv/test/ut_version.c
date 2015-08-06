#include "ut_public.h"



void case_version()
{
        CU_ASSERT_PTR_NOT_NULL(kv_version());
        CU_ASSERT_STRING_EQUAL(kv_version(), "0.0.2");
        CU_ASSERT_EQUAL(kv_version_numeric(), 000002L);
}



#ifndef ENABLE_AUTO

int main(int argc, char *argv[])
{
    UTIL_INIT_DEFAULT();

    UTIL_ADD_CASE("case_version()", case_version);

    UTIL_RUN();
    UTIL_UNINIT();
    
    return 0;
}

#endif
