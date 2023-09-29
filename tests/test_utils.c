#include "test.h"

#include <stdlib.h>
#include <stdio.h>
#include <check.h>

#include <sys/resource.h>
#include <errno.h>

#include "../src/utils.h"


/* helper function to lower RLIMIT_DATA to test out of memory conditions
 * returns old limit
 * Very helpfully does not work on MacOS; will simply return error 22 (EINVAL)
 * Can't find any documentation about that apart from it being a known issue
 */
#ifndef __APPLE__
rlim_t set_mem_limit(rlim_t limit)
{
	struct rlimit old_rlim, new_rlim;

	/* fetch old limit */
	if (getrlimit(RLIMIT_DATA, &old_rlim) != 0) {
		fprintf(stderr, "getrlimit: %i, %s\n", errno, strerror(errno));
		exit(1);
	}

	/* set new limit */
	new_rlim.rlim_cur = limit;
	new_rlim.rlim_max = old_rlim.rlim_max;
	if (setrlimit(RLIMIT_DATA, &new_rlim) != 0) {
		fprintf(stderr, "setrlimit: %i, %s\n", errno, strerror(errno));
		exit(1);
	}

	return old_rlim.rlim_cur;
}


void fill_memory(bool fill)
{
	static rlim_t old_rlim = -1;
	static void *buf = NULL;

	if (fill)
	{
		/* limit memory we can allocate*/
		old_rlim = set_mem_limit(2048*1024);

        /* make sure buf is not allocated */
		if (buf)
		{
			free(buf);
			buf = NULL;
		}

		/* exhaust memory */
		size_t len = 512*1024;
		while ((buf = malloc(len))!=NULL) {
			free(buf);
			len += 4096;
		}
		fprintf(stderr, "len is %zu\n", len);
	}
	else
	{
		/* reset rlimit */
		if (old_rlim>0)
			set_mem_limit(old_rlim);

		/* cleanup */
		if (buf)
		{
			free(buf);
			buf = NULL;
		}
	}
}

void unfill_memory(void)
{
	fill_memory(false);
}

#endif

const char json_txt[] = "{                      " \
	"\"key_true\": true,                        " \
	"\"key_false\": false,                      " \
	"\"key_int\": 1,                            " \
	"\"key_str\": \"test\",                     " \
	"\"key_str_empty\": \"\",                   " \
	"\"key_obj\": { \"key_sub\": \"val_sub\" }  " \
"}";



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

START_TEST (test_str_printf_oom)
{
#ifdef __APPLE__
#  pragma message "out-of-memory test disabled on MacOS"
#else
	char *str = malloc(1024*1024);
	memset(str, 'x', 1024*1024);
	str[1024*1024-1]='\0';

	/* check failure mode str too large */
	fill_memory(true);
	ck_assert_ptr_eq(str_printf("foo: %s", str), NULL);
	unfill_memory();

	/* cleanup */
	free(str);
#endif
}
END_TEST

START_TEST(test_json_utils)
{
	json_value *json = json_parse(json_txt, strlen(json_txt));
	ck_assert_ptr_ne(json, NULL);

	/* matching nothing */
	ck_assert_ptr_eq(NULL, findKey(NULL, "key_str"));
	ck_assert_ptr_eq(NULL, findKey(json, NULL));
	ck_assert_ptr_eq(NULL, findKey(json, ""));

	/* nonexisting key */
	ck_assert_ptr_eq(NULL, findKey(json, "bestaat_niet"));
	ck_assert_ptr_eq(NULL, findKey(json, "key_obj."));

	/* lookup a key */
	json_value *result = findKey(json, "key_str");
	ck_assert_ptr_ne(result, NULL);
	ck_assert_str_eq(result->u.string.ptr, "test");

	/* lookup a nested key */
	json_value *result2 = findKey(json, "key_obj.key_sub");
	ck_assert_ptr_ne(result2, NULL);
	ck_assert_str_eq(result2->u.string.ptr, "val_sub");

	/* convenience function to get string or bool */
	ck_assert_int_eq(getBool(json, "key_true"), true);
	ck_assert_int_eq(getBool(json, "key_false"), false);
	ck_assert_int_eq(getBool(json, "bestaat_niet"), false);

	ck_assert_str_eq(getString(json, "key_str"), "test");
	ck_assert_str_eq(getString(json, "key_str_empty"), "");
	ck_assert_ptr_eq(getString(json, "bestaat_niet"), NULL);

	/* cleanup */
	json_value_free(json);
}
END_TEST

START_TEST (test_json_utils_oom)
{
#ifdef __APPLE__
#  pragma message "out-of-memory test disabled on MacOS"
#else
	json_value *json = json_parse(json_txt, strlen(json_txt));
	ck_assert_ptr_ne(json, NULL);

	char *key = malloc(1024*1024);
	memset(key, 'x', 1024*1024);
	key[1024*1024-1]='\0';


	/* check failure mode (key too large) */
	fill_memory(true);
	ck_assert_ptr_eq(NULL, findKey(json, key));
	unfill_memory();

	/* cleanup */
	free(key);
	json_value_free(json);

#endif
}
END_TEST

TCase * test_utils(void)
{
    TCase *tc =tcase_create("config");

	tcase_set_timeout(tc, 7200);

	/* trim() */
    tcase_add_test(tc, test_trim_ok);
    tcase_add_test(tc, test_trim_fail);

	/* str_printf() */
    tcase_add_test(tc, test_str_printf);
    tcase_add_test(tc, test_str_printf_oom);

	/* json utils */
    tcase_add_test(tc, test_json_utils);
	tcase_add_test(tc, test_json_utils_oom);

    return tc;
}
