#include "ut_public.h"




void case_cmd_sismember()
{

}




#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE("case_cmd_sismember()", case_cmd_sismember);
        UTIL_RUN();
        UTIL_UNINIT();

        return 0;
}
#endif
