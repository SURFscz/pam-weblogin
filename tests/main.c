#include "test.h"

#include <stdlib.h>
#include <stdio.h>
#include <check.h>


Suite * test_suite(void)
{
    Suite *s = suite_create("pam_weblogin");
    suite_add_tcase(s, test_utils());
    return s;
}

int main(void)
{
    Suite* s = test_suite();
    SRunner* sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    int number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

