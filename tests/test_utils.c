#include "test.h"

#include <stdlib.h>
#include <stdio.h>
#include <check.h>

#include <sys/resource.h>
#include <errno.h>

#include "../src/utils.h"


/* test the trim() function in utils.c */
START_TEST (test_trim_ok)
{
	char s1[8] = "  foo  ";
	ck_assert_str_eq(trim(s1), "foo");

	char s2[8] = "  foo";
	ck_assert_str_eq(trim(s2), "foo");

	char s3[8] = "foo  ";
	ck_assert_str_eq(trim(s3), "foo");

	char s4[8] = "foo";
	ck_assert_str_eq(trim(s4), "foo");

	char s5[8] = "      ";
	ck_assert_str_eq(trim(s5), "");
}
END_TEST

START_TEST (test_trim_fail)
{
	char s1[8] = "";
	ck_assert_str_eq(trim(s1), "");

	ck_assert( trim(NULL)==NULL );
}
END_TEST

START_TEST (test_str_printf)
{
	char *s1 = str_printf("foo: %i", 42);
	ck_assert_str_eq(s1, "foo: 42");
	free(s1);

	char *s2 = str_printf((char*) NULL);
	ck_assert(s2 == NULL);
}
END_TEST

TCase * test_config(void)
{
    TCase *tc =tcase_create("config");

	/* add tests to testcase */
    tcase_add_test(tc, test_trim_ok);
    tcase_add_test(tc, test_trim_fail);
    tcase_add_test(tc, test_str_printf);

    return tc;
}
