/*
* Copyright (c) 2009, Björn Rehm (bjoern@shugaa.de)
* All rights reserved.
* 
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* 
*  * Redistributions of source code must retain the above copyright notice, this
*    list of conditions and the following disclaimer.
*  * Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*  * Neither the name of the author nor the names of its contributors may be
*    used to endorse or promote products derived from this software without
*    specific prior written permission.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

    strncpy(buf, "plot '-' using 1:($2) '%lf %lf' smooth unique notitle axes x1y1\n1 2\n2 3\ne\n", sizeof(buf));
    len = strlen(buf);
    rc = gpif_write(&session, (const char*)buf, &len);
    CU_ASSERT(rc == EGPIFOK);

    len = sizeof(buf)-1;
    rc = gpif_read(&session, buf, &len);
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
