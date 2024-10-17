#include "test.h"

#include <stdlib.h>
#include <stdio.h>
#include <check.h>

#include <sys/resource.h>
#include <errno.h>

#include "../src/utils.h"
#include "../src/tty.h"


/* helper function to lower RLIMIT_DATA to test out of memory conditions
 * returns old limit
 * Very helpfully does not work on MacOS; will simply return error 22 (EINVAL)
 * Can't find any documentation about that apart from it being a known issue
 */
#ifndef __APPLE__
rlim_t set_mem_limit(rlim_t limit)
{
	struct rlimit old_rlim, new_rlim;
	int result;

	/* fetch old limit */
	result = getrlimit(RLIMIT_DATA, &old_rlim);
	ck_assert_int_eq(result, 0);

	/* set new limit */
	new_rlim.rlim_cur = limit;
	new_rlim.rlim_max = old_rlim.rlim_max;
	result = setrlimit(RLIMIT_DATA, &new_rlim);
	ck_assert_int_eq(result, 0);

	return old_rlim.rlim_cur;
}


void fill_memory(bool fill)
{
	const size_t mem_max = 2048*1024;
	const size_t mem_step = 4096-16; /* glibc-specific?, see https://stackoverflow.com/questions/62095835 */
	static rlim_t old_rlim = -1;
	static void** buf = NULL;

	if (fill)
	{
		/* limit memory we can allocate*/
		old_rlim = set_mem_limit(mem_max);
		fprintf(stderr, "setting old rlimit to %zd\n", old_rlim);

        /* make sure buf is not allocated */
		ck_assert_ptr_null(buf);

		/* we are going to allocate a number of buffers to exhaust the memory */
		/* we keep the address of the previous buffer in the first pointer of the next one */
		/* allocate first buffer */
		buf = malloc(mem_step);
		ck_assert_ptr_nonnull(buf);

		/* there is no previous buffer */
		*buf = NULL;

		size_t num = 0;
		while (1)
		{
			// fprintf(stderr, "allocate buffer number %zu\n", num);

			/* allocate next buffer */
			void **next = malloc(mem_step);

			/* is memory exhausted already? */
			if (next==NULL)
			{
				fprintf(stderr, "malloc failed after %zu buffers\n", num);
				break;
			}

			/* store the address of the previous buf in the new one */
			*next = buf;
			/* and set the buffer itself to the next one */
			buf = next;

			num++;

			/* check if we are not allocating too much */
			ck_assert_int_lt(num * mem_step, mem_max*4);
		}
	}
	else
	{
		fprintf(stderr, "resetting memory limits\n");

		/* reset rlimit */
		set_mem_limit(old_rlim);

		/* cleanup */
		ck_assert_ptr_ne(buf, NULL);

		/* loop over all previously allocated buffers */
		while (*buf != NULL)
		{
			/* fetch the address of the previous buffer */
			void **next = *buf;
			/* free the current buffer */
			free(buf);
			/* move on the the next buffer */
			buf = next;
		}
		/* free the final buffer */
		free(buf);
		buf = NULL;
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
	"\"key_obj\": { \"key_sub\": \"val_sub\" }, " \
	"\"key_array\": [ \"one\", \"two\" ]        " \
"}";



/* test the trim() function in utils.c */
START_TEST (test_trim_ok)
{
	char s1[8] = "  foo  ";
	ck_assert_str_eq(trim(s1, 8), "foo");

	char s2[8] = "  foo";
	ck_assert_str_eq(trim(s2, 8), "foo");

	char s3[8] = "foo  ";
	ck_assert_str_eq(trim(s3, 8), "foo");

	char s4[8] = "foo";
	ck_assert_str_eq(trim(s4, 8), "foo");

	char s5[8] = "      ";
	ck_assert_str_eq(trim(s5, 8), "");
}
END_TEST

START_TEST (test_trim_fail)
{
	char s1[8] = "";
	ck_assert_str_eq(trim(s1, 8), "");

	ck_assert( trim(NULL, 8)==NULL );
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
	json_value *result1 = findKey(json, "key_str");
	ck_assert_ptr_ne(result1, NULL);
	ck_assert_str_eq(result1->u.string.ptr, "test");

	/* lookup a nested key */
	json_value *result2 = findKey(json, "key_obj.key_sub");
	ck_assert_ptr_ne(result2, NULL);
	ck_assert_str_eq(result2->u.string.ptr, "val_sub");

	/* get first key */
	char *result3 = getKey(findKey(json, "key_obj"), 0);
	ck_assert_str_eq(result3, "key_sub");

	/* get first value */
	char *result4 = getValue(findKey(json, "key_obj"), 0);
	ck_assert_str_eq(result4, "val_sub");

	/* get first index */
	json_value *item1 = getIndex(findKey(json, "key_array"), 0);
	char *result5 = item1->u.string.ptr;
	ck_assert_str_eq(result5, "one");

	/* get second index */
	json_value *item2 = getIndex(findKey(json, "key_array"), 1);
	char *result6 = item2->u.string.ptr;
	ck_assert_str_eq(result6, "two");

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

START_TEST (test_input_is_safe)
{
	ck_assert_int_eq(input_is_safe("", 0), 1);
	ck_assert_int_eq(input_is_safe("A", 0), 0);
	ck_assert_int_eq(input_is_safe("ABC", 8), 1);
	ck_assert_int_eq(input_is_safe("123456789", 8), 0);
	ck_assert_int_eq(input_is_safe("AB.C", 8), 0);
        ck_assert_int_eq(input_is_safe("\"ABC\"", 8), 0);
        ck_assert_int_eq(input_is_safe("A\"}BBB", 8), 0);
        ck_assert_int_eq(input_is_safe("1234", 3), 0);
        ck_assert_int_eq(input_is_safe("1234", 4), 1);
}
END_TEST

TCase * test_utils(void)
{
    TCase *tc =tcase_create("utils");

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
