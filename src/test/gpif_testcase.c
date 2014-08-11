#include <stdio.h>
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <CUnit/Automated.h>
#include "gpif.h"

#define CU_ADD_TEST(suite, test) (CU_add_test(suite, #test, (CU_TestFunc)test))

static const char cmd[] = "plot '-' with lines smooth unique notitle axes x1y1\n1 2\n2 3\ne\n";

/* Test libgpif functionality. There's not much to test at the moment anyway */
static void test_gpif(void) 
{
    int rc;
    size_t len;
    
    gpif_session session;

    char *const argv[] = {
        "gnuplot",
        "-noraise",
        NULL
    };

    rc = gpif_init(&session, argv);
    CU_ASSERT(rc == EGPIFOK);

    len = strlen(cmd);
    rc = gpif_write(&session, cmd, &len);
    CU_ASSERT(rc == EGPIFOK);

    rc = gpif_close(&session);
    CU_ASSERT(rc == EGPIFOK);
}

int main(int argc, char *argv[]) 
{
    int ret = 0;
    CU_ErrorCode cu_rc;
    CU_pSuite cu_suite01;
    CU_pTest cu_test;

    for (;;)
    {
        /* Initialize test registry */
        cu_rc = CU_initialize_registry();
        if (cu_rc != CUE_SUCCESS) {
            ret = 1;
            break;
        }

        /* Add a suite to the registry */
        cu_suite01 = CU_add_suite("gpiftest", NULL, NULL);
        if (cu_suite01 == NULL) {
            ret = 2;
            break;
        }

        /* Add tests to suite */
        cu_test = CU_ADD_TEST(cu_suite01, test_gpif);
        if (cu_test == NULL) {
            ret = 3;
            break;
        }

        /* Be verbose */
        CU_basic_set_mode(CU_BRM_VERBOSE);

        /* Run all tests in our suite */
        cu_rc = CU_basic_run_tests();
        if (cu_rc != CUE_SUCCESS) {
            ret = 4;
            break;
        }

        break;
    }

    switch (ret) {
        case 1:
            printf("CU_initialize_registry() failed\n");
            break;
        case 2:
            printf("CU_add_suite() failed\n");
            break;
        case 3:
            printf("CU_ADD_TEST() failed\n");
            break;
        case 4:
            printf("CU_basic_run_tests() failed\n");
            break;
        default:
            break;
    }

    CU_cleanup_registry();
    return ret;
}

