#include "kv_inner.h"
#include "ut_public.h"


#define ERR_TABLE_STEP 5
static int err_table_size = 0;
static char **err_table;


static void err_table_add(int errnum, const char* desc)
{
        if (err_table_size == 0) {
                err_table = zmalloc(sizeof(char*)*ERR_TABLE_STEP);
                err_table_size = ERR_TABLE_STEP;
        }

        if (errnum >= err_table_size) {
                int incr = err_table_size;
                char **old = err_table;
                while(errnum >= incr) {
                        incr += ERR_TABLE_STEP;
                }
                err_table = zrealloc(err_table, sizeof(char*)*incr);
                err_table_size = incr;
        }

        err_table[errnum] = strdup(desc);
}

const char* err_table_get(int errnum)
{
        return err_table[errnum];
}

void err_table_create()
{
        err_table_add(ERR_NONE, "no error");
        err_table_add(ERR_TYPE, "-ERR wrong type");
        err_table_add(ERR_NO_KEY, "-ERR no such key");
        err_table_add(ERR_SYNTAX, "-ERR syntax error");
        err_table_add(ERR_SAME_OBJECT, "-ERR source and destination objects are the same");
        err_table_add(ERR_OUT_OF_RANGE, "-ERR index out of range");
        err_table_add(ERR_NIL, "-ERR nil");
        err_table_add(ERR_NOT_INIT, "-ERR not init");
        err_table_add(ERR_ARGUMENTS, "-ERR wrong number of arguments");
        err_table_add(ERR_PROTOCOL, "-ERR protocol error");
        err_table_add(ERR_VALUE, "-ERR value invalid");
}

void err_table_release()
{
        zfree(err_table);
}





void test_error()
{
        err_table_create();

        printf("errnum:%d :%s\n", ERR_NONE, err_table_get(ERR_NONE));
        printf("errnum:%d :%s\n", ERR_TYPE ,err_table_get(ERR_TYPE));
        printf("errnum:%d :%s\n", ERR_NO_KEY, err_table_get(ERR_NO_KEY));
        printf("errnum:%d :%s\n", ERR_SYNTAX, err_table_get(ERR_SYNTAX));
        printf("errnum:%d :%s\n", ERR_SAME_OBJECT, err_table_get(ERR_SAME_OBJECT));
        printf("errnum:%d :%s\n", ERR_OUT_OF_RANGE, err_table_get(ERR_OUT_OF_RANGE));
        printf("errnum:%d :%s\n", ERR_NIL, err_table_get(ERR_NIL));
        printf("errnum:%d :%s\n", ERR_NOT_INIT, err_table_get(ERR_NOT_INIT));
        printf("errnum:%d :%s\n", ERR_ARGUMENTS, err_table_get(ERR_ARGUMENTS));
        printf("errnum:%d :%s\n", ERR_PROTOCOL, err_table_get(ERR_PROTOCOL));
        printf("errnum:%d :%s\n", ERR_VALUE, err_table_get(ERR_VALUE));

        err_table_release();
}







#ifndef UT_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE("test_error()", test_error);
        UTIL_RUN();
        UTIL_UNINIT();

        return 0;
}
#endif
