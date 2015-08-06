#include "ut_public.h"




void test_zadd()
{

}




#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE("test_zadd()", test_zadd);
        UTIL_RUN();
        UTIL_UNINIT();

        return 0;
}
#endif
