/***
  This file is part of PulseAudio.

  PulseAudio is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; either version 2.1 of the License,
  or (at your option) any later version.

  PulseAudio is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with PulseAudio; if not, see <http://www.gnu.org/licenses/>.
***/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <check.h>

#include <pulsecore/modargs.h>


START_TEST (modargs_test_null) {
    const char* keys[] = {
        NULL
    };
    pa_modargs *args;

    args = pa_modargs_new(NULL, keys);
    ck_assert_ptr_ne(args, NULL);

    pa_modargs_free(args);
}
END_TEST

START_TEST (modargs_test_empty_string) {
    const char* keys[] = {
        NULL
    };
    pa_modargs *args;

    args = pa_modargs_new("", keys);
    ck_assert_ptr_ne(args, NULL);

    pa_modargs_free(args);
}
END_TEST

START_TEST (modargs_test_get_value_1) {
    const char* keys[] = {
        "abc",
        NULL
    };
    pa_modargs *args;
    const char* value;

    args = pa_modargs_new("", keys);
    ck_assert_ptr_ne(args, NULL);

    /* default value handling */
    value = pa_modargs_get_value(args, "abc", NULL);
    ck_assert_ptr_eq(value, NULL);

    /* default value handling */
    value = pa_modargs_get_value(args, "abc", "default value");
    ck_assert_str_eq(value, "default value");

    /* unknown key handling */
    value = pa_modargs_get_value(args, "def", "default value");
    ck_assert_str_eq(value, "default value");

    pa_modargs_free(args);
}
END_TEST

START_TEST (modargs_test_get_value_2) {
    const char* keys[] = {
        "abc",
        "def",
        "ghi",
        NULL
    };
    pa_modargs *args;
    const char* value;

    args = pa_modargs_new("abc=123 def=xyz", keys);
    ck_assert_ptr_ne(args, NULL);

    /* regular value handling */
    value = pa_modargs_get_value(args, "abc", "default value");
    ck_assert_str_eq(value, "123");

    /* regular value handling */
    value = pa_modargs_get_value(args, "def", "default value");
    ck_assert_str_eq(value, "xyz");

    /* default value handling */
    value = pa_modargs_get_value(args, "ghi", "default value");
    ck_assert_str_eq(value, "default value");

    /* unknown key handling */
    value = pa_modargs_get_value(args, "jkl", "default value");
    ck_assert_str_eq(value, "default value");

    pa_modargs_free(args);
}
END_TEST

int main(int argc, char *argv[]) {
    int failed = 0;
    Suite *s;
    TCase *tc;
    SRunner *sr;

    if (!getenv("MAKE_CHECK"))
        pa_log_set_level(PA_LOG_DEBUG);

    s = suite_create("Modargs");
    tc = tcase_create("modargs");
    tcase_add_test(tc, modargs_test_null);
    tcase_add_test(tc, modargs_test_empty_string);
    tcase_add_test(tc, modargs_test_get_value_1);
    tcase_add_test(tc, modargs_test_get_value_2);
    suite_add_tcase(s, tc);

    sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
