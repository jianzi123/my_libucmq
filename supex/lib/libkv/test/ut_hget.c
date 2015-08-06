#include "ut_public.h"



void test_hget()
{

}




#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE("test_hget()", test_hget);
        UTIL_RUN();
        UTIL_UNINIT();

        return 0;
}
#endif
