#include "test.h"

#include <stdlib.h>
#include <stdio.h>
#include <check.h>

#include <sys/resource.h>
#include <errno.h>

#include "../src/config.h"

START_TEST (test_getConfig)
{
	/* Read configuration file */
	Config *cfg = NULL;
	cfg = getConfig("pam-weblogin.conf");
	ck_assert_ptr_ne(cfg, NULL);
	ck_assert_str_eq(cfg->url, "https://sbs.scz-vm.net/pam-weblogin");
	ck_assert_str_eq(cfg->token, "Bearer test");
	ck_assert_int_eq(cfg->retries, 3);
	ck_assert_str_eq(cfg->attribute, "uid");
	ck_assert_int_eq(cfg->cache_duration, 60);
	ck_assert(cfg->pam_user);
}
END_TEST

TCase * test_config(void)
{
    TCase *tc =tcase_create("config");

	tcase_set_timeout(tc, 7200);

	/* getConfig() */
    tcase_add_test(tc, test_getConfig);

	return tc;
}
