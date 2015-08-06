#include "CUnit/Basic.h"
#include <stdio.h>

static FILE* temp_file = NULL;
int init_suite1()
{
    printf("=== init_suite ===");
    return 0;
}

int clean_suite1()
{
    printf("=== clean_suite ===");
    return 0;
}

void testF()
{
    printf("=== test1 ===");
    CU_ASSERT(0);
    CU_ASSERT(0);
}

void testQ()
{
    printf("=== test2 ===");
    CU_ASSERT(1);
}



int main(int argc, char *argv[])
{
    CU_pSuite suite = NULL;

    if (CU_initialize_registry() != CUE_SUCCESS)
        return CU_get_error();

    suite = CU_add_suite("Suite_1", init_suite1, clean_suite1);
    if (!suite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (!CU_add_test(suite, "test1", testF) ||
        !CU_add_test(suite, "test2", testQ)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();

    return CU_get_error();
}
