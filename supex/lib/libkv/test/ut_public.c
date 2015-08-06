#include "ut_public.h"


static int init_default(void)
{
        return 0;
}

static int clean_default(void)
{
        return 0;
}


static CU_pSuite util_suite = NULL;

void UTIL_INIT(init_handler init, init_handler clean)
{
    assert(CU_initialize_registry() == CUE_SUCCESS);
    assert((util_suite = CU_add_suite("Util_suite", init, clean)) != NULL);
}

void UTIL_INIT_DEFAULT()
{
    UTIL_INIT(init_default, clean_default);
}

void UTIL_ADD_CASE(const char* desc, case_handler handler)
{
    assert(CU_add_test(util_suite, desc, handler));
}

void UTIL_RUN()
{
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
}

void UTIL_UNINIT()
{
    CU_cleanup_registry();
}

void answer_print(answer_t *a)
{
        answer_iter_t *iter;
        answer_value_t *value;
        
        printf("answer session ===>:\n");
        if (a->errnum != ERR_NONE) {
                printf("error:%s\n", a->err);
        } else {
                iter = answer_get_iter(a, ANSWER_HEAD);
                while((value = answer_next(iter)) != NULL) {
                        printf("===> size:%ld value:%s\n", value->ptrlen, (char*)value->ptr);
                }
                answer_release_iter(iter);
        }  
}

void answer_print_release(answer_t *a)
{
        if (!a) return;
        
        answer_print(a);
        answer_release(a);
}
