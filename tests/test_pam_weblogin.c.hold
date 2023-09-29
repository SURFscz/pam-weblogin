#include "test.h"

#include <stdlib.h>
#include <stdio.h>
#include <check.h>

#include <sys/resource.h>
#include <errno.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>

#include "../src/pam_weblogin.h"

/* pam_conv conversation helper function */
int converse(UNUSED int n, UNUSED const struct pam_message **msg,
    UNUSED struct pam_response **resp, UNUSED void *data)
{
	return PAM_SUCCESS;
}

START_TEST (test_pam_sm_funcs)
{
	/* Initialise PAM */
	int pamr = 0;
	struct pam_conv conv;
	conv.conv = converse;
	conv.appdata_ptr = NULL;
	pam_handle_t *pamh = NULL;
	pamr = pam_start("weblogin", "user", &conv, &pamh);
	ck_assert_int_eq(pamr, PAM_SUCCESS);

	const char *argv[] = {"pam-weblogin.conf"};
	int result = 0;

	/* pam_sm_setcred */
	result = pam_sm_setcred(pamh, 0, 1, argv);
	ck_assert_int_eq(result, PAM_SUCCESS);

	/* pam_sm_acct_mgmt */
	result = pam_sm_acct_mgmt(pamh, 0, 1, argv);
	ck_assert_int_eq(result, PAM_SUCCESS);

	/* pam_sm_authenticate */
	result = pam_sm_authenticate(pamh, 0, 1, argv);
	// Fails on unreachable server
	ck_assert_int_eq(result, PAM_SYSTEM_ERR);
}
END_TEST

TCase * test_pam_weblogin(void)
{
    TCase *tc =tcase_create("pam-weblogin");

	tcase_set_timeout(tc, 7200);

	/* Test pam_sm_ functions */
    tcase_add_test(tc, test_pam_sm_funcs);

	return tc;
}
