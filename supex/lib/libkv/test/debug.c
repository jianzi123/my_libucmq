#include "ut_public.h"



void debug_memory_leak()
{
        answer_t *ans;
        answer_iter_t *iter;
        answer_value_t *value;
        int i;

        
        kv_init(NULL);


        ans = kv_ask("set key", 7+2);
        printf("errnum:%d\n", ans->errnum);
        printf("err:%s\n", ans->err);
        answer_release(ans);
        
        kv_uninit();
}

void debug_random()
{
        int i;
        for (i = 5; i <= 100; i++) {
                printf("ran(%d):%s\n", i, generator_rand_str(i)->ptr);
        }
}


int main(int argc, char *argv[])
{


//        debug_memory_leak();
        debug_random();

        
        return 0;
}

