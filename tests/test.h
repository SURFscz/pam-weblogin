#include "../src/defs.h"

#include <check.h>

#ifndef ck_assert_ptr_null /* check.h version < 0.14.0 */
# define ck_assert_ptr_null(ptr) ck_assert_ptr_eq((ptr), NULL)
#endif

#ifndef ck_assert_ptr_nonnull /* check.h version < 0.14.0 */
# define ck_assert_ptr_nonnull(ptr) ck_assert_ptr_ne((ptr), NULL)
#endif

TCase * test_utils(void);
TCase * test_config(void);
TCase * test_pam_weblogin(void);
