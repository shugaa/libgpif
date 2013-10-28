/*
* Copyright (c) 2008, Björn Rehm (bjoern@shugaa.de)
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/

#include <stdio.h>
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <CUnit/Automated.h>

#include "gpif.h"

#define CU_ADD_TEST(suite, test) (CU_add_test(suite, #test, (CU_TestFunc)test))

/* Test libgpif functionality. There's not much to test at the moment anyway */
static void test_gpif(void) 
{
    int rc;
    size_t len;
    char buf[255];
    gpif_session_t session;

    char *const argv[] = {
        "gnuplot",
        "-noraise",
        NULL
    };

    rc = gpif_init(&session, argv);
    CU_ASSERT(rc == EGPIFOK);

    strncpy(buf, "plot '-' with lines smooth unique notitle axes x1y1\n1 2\n2 3\ne\n", sizeof(buf));
    len = strlen(buf);
    rc = gpif_write(&session, (const char*)buf, &len);
    CU_ASSERT(rc == EGPIFOK);

    /* Usually there is nothing to read */
#if 0
    len = sizeof(buf)-1;
    rc = gpif_read(&session, buf, &len);
    CU_ASSERT(rc == EGPIFOK);
#endif

    rc = gpif_close(&session);
    CU_ASSERT(rc == EGPIFOK);
}

int main(int argc, char *argv[]) 
{
    int ret = 0;
    CU_ErrorCode cu_rc;
    CU_pSuite cu_suite01;
    CU_pTest cu_test;

    /* Initialize test registry */
    cu_rc = CU_initialize_registry();
    if (cu_rc != CUE_SUCCESS) {
        ret = 1;
        goto finish;
    }

    /* Add a suite to the registry */
    cu_suite01 = CU_add_suite("gpiftest", NULL, NULL);
    if (cu_suite01 == NULL) {
        ret = 2;
        goto finish;
    }

    /* Add tests to suite */
    cu_test = CU_ADD_TEST(cu_suite01, test_gpif);
    if (cu_test == NULL) {
        ret = 3;
        goto finish;
    }

    /* Be verbose */
    CU_basic_set_mode(CU_BRM_VERBOSE);

    /* Run all tests in our suite */
    cu_rc = CU_basic_run_tests();
    if (cu_rc != CUE_SUCCESS) {
        ret = 4;
        goto finish;
    }

finish:
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
