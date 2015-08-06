#include "ut_public.h"



void test_zrange()
{
 
}




#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE("test_zrange()", test_zrange);
        UTIL_RUN();
        UTIL_UNINIT();

        return 0;
}
#endif
