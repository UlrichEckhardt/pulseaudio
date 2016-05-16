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


// test calling pa_modargs_new(NULL, ..)
START_TEST (modargs_test_null) {
    const char* keys[] = {
        NULL
    };
    pa_modargs *args;

    args = pa_modargs_new(NULL, keys);
    ck_assert(args != NULL);

    pa_modargs_free(args);
}
END_TEST

// test calling pa_modargs_new("", ..)
START_TEST (modargs_test_empty_string) {
    const char* keys[] = {
        NULL
    };
    pa_modargs *args;

    args = pa_modargs_new("", keys);
    ck_assert(args != NULL);

    pa_modargs_free(args);
}
END_TEST

// test calling pa_modargs_get_value("", ..)
START_TEST (modargs_test_get_value_1) {
    const char* keys[] = {
        "abc",
        NULL
    };
    pa_modargs *args;
    const char* value;

    args = pa_modargs_new("", keys);
    ck_assert(args != NULL);

    // default value handling
    value = pa_modargs_get_value(args, "abc", NULL);
    ck_assert(value == NULL);

    // default value handling
    value = pa_modargs_get_value(args, "abc", "default value");
    ck_assert(value != NULL);
    ck_assert_str_eq(value, "default value");

    // unknown key handling
    value = pa_modargs_get_value(args, "def", "default value");
    ck_assert(value != NULL);
    ck_assert_str_eq(value, "default value");

    pa_modargs_free(args);
}
END_TEST

// test calling pa_modargs_get_value()
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
    ck_assert_ptr_ne(NULL, args);

    value = pa_modargs_get_value(args, "abc", "default value");
    ck_assert_str_eq(value, "123");

    value = pa_modargs_get_value(args, "def", "default value");
    ck_assert_str_eq(value, "xyz");

    value = pa_modargs_get_value(args, "def", "default value");
    ck_assert_str_eq(value, "xyz");

    pa_modargs_free(args);
}
END_TEST

// test pa_modargs_get_value_u32()
START_TEST (modargs_test_get_value_u32) {
    const char* keys[] = {
        "abc",
        "def",
        NULL
    };
    pa_modargs *args;
    uint32_t value;

    args = pa_modargs_new("abc=123 def=xyz", keys);
    ck_assert(args != NULL);

    // test extracting and parsing
    ck_assert_int_eq(pa_modargs_get_value_u32(args, "abc", &value), 0);
    ck_assert_int_eq(value, 123);

    // test extracting and parsing failure
    ck_assert_int_lt(pa_modargs_get_value_u32(args, "def", &value), 0);

    // test extracting an undefined value
    // TODO: this test fails to signal failure
    // ck_assert_int_lt(pa_modargs_get_value_u32(args, "ghi", &value), 0);

    pa_modargs_free(args);
}
END_TEST

// test pa_modargs_get_value_s32()
START_TEST (modargs_test_get_value_s32) {
    const char* keys[] = {
        "abc",
        "def",
        NULL
    };
    pa_modargs *args;
    int32_t value;

    args = pa_modargs_new("abc=-123 def=xyz", keys);
    ck_assert(args != NULL);

    // test extracting and parsing
    ck_assert_int_eq(pa_modargs_get_value_s32(args, "abc", &value), 0);
    ck_assert_int_eq(value, -123);

    // test extracting and parsing failure
    ck_assert_int_lt(pa_modargs_get_value_s32(args, "def", &value), 0);

    // test extracting an undefined value
    // TODO: this test fails to signal failure
    // ck_assert_int_lt(pa_modargs_get_value_s32(args, "ghi", &value), 0);

    pa_modargs_free(args);
}
END_TEST

// test pa_modargs_get_value_boolean()
START_TEST (modargs_test_get_value_boolean) {
    const char* keys[] = {
        "abc",
        "def",
        NULL
    };
    pa_modargs *args;
    bool value;

    args = pa_modargs_new("abc=true def=whatever", keys);
    ck_assert_ptr_ne(args, NULL);

    // test extracting and parsing
    ck_assert_int_eq(pa_modargs_get_value_boolean(args, "abc", &value), 0);
    ck_assert(value == true);

    // test extracting and parsing failure
    ck_assert_int_lt(pa_modargs_get_value_boolean(args, "def", &value), 0);

    // test extracting an undefined value
    // TODO: this test fails to signal failure
    // ck_assert_int_lt(pa_modargs_get_value_boolean(args, "ghi", &value), 0);

    pa_modargs_free(args);
}
END_TEST

// test pa_modargs_get_value_double()
START_TEST (modargs_test_get_value_double) {
    const char* keys[] = {
        "abc",
        "def",
        NULL
    };
    pa_modargs *args;
    double value;

    args = pa_modargs_new("abc=12.25 def=whatever", keys);
    ck_assert_ptr_ne(args, NULL);

    // test extracting and parsing
    ck_assert_int_eq(pa_modargs_get_value_double(args, "abc", &value), 0);
    ck_assert(value <= 12.26);
    ck_assert(value >= 12.24);

    // test extracting and parsing failure
    ck_assert_int_lt(pa_modargs_get_value_double(args, "def", &value), 0);

    // test extracting an undefined value
    // TODO: this test fails to signal failure
    // ck_assert_int_lt(pa_modargs_get_value_double(args, "ghi", &value), 0);

    pa_modargs_free(args);
}
END_TEST

// test pa_modargs_get_value_volume()
START_TEST (modargs_test_get_value_volume) {
    const char* keys[] = {
        "valueA",
        "valueZ",
        NULL
    };
    pa_modargs *args;
    pa_volume_t value;

    args = pa_modargs_new("valueA=12.25dB valueZ=aoeui", keys);
    ck_assert_ptr_ne(args, NULL);

    // test extracting and parsing
    ck_assert_int_eq(pa_modargs_get_value_volume(args, "valueA", &value), 0);
    ck_assert_int_eq(value, 104869);

    // test extracting and parsing failure
    ck_assert_int_lt(pa_modargs_get_value_volume(args, "valueZ", &value), 0);

    // test extracting an undefined value
    // TODO: this test fails to signal failure
    // ck_assert_int_lt(pa_modargs_get_value_double(args, "ghi", &value), 0);

    pa_modargs_free(args);
}
END_TEST

// test pa_modargs_get_sample_rate
START_TEST (modargs_test_get_sample_rate_1) {
    const char* keys[] = {
        "rate",
        NULL
    };
    pa_modargs *args;
    uint32_t value;

    args = pa_modargs_new("rate=1225", keys);
    ck_assert_ptr_ne(args, NULL);

    ck_assert_int_eq(pa_modargs_get_sample_rate(args, &value), 0);
    ck_assert_int_eq(value, 1225);

    pa_modargs_free(args);
}
END_TEST

// test pa_modargs_get_sample_rate for parsing failure
START_TEST (modargs_test_get_sample_rate_2) {
    const char* keys[] = {
        "rate",
        NULL
    };
    pa_modargs *args;
    uint32_t value;

    args = pa_modargs_new("rate=whatever", keys);
    ck_assert_ptr_ne(args, NULL);

    ck_assert_int_lt(pa_modargs_get_sample_rate(args, &value), 0);

    pa_modargs_free(args);
}
END_TEST

// test pa_modargs_get_sample_rate for invalid rate detection
START_TEST (modargs_test_get_sample_rate_3) {
    const char* keys[] = {
        "rate",
        NULL
    };
    pa_modargs *args;
    uint32_t value;

    args = pa_modargs_new("rate=0", keys);
    ck_assert_ptr_ne(args, NULL);

    ck_assert_int_lt(pa_modargs_get_sample_rate(args, &value), 0);

    pa_modargs_free(args);
}
END_TEST

// test pa_modargs_get_sample_spec
START_TEST (modargs_test_get_sample_spec_1) {
    const char* keys[] = {
        "rate",
        "format",
        "channels",
        NULL
    };
    pa_modargs *args;
    pa_sample_spec value;

    args = pa_modargs_new("rate=1234 format=s16be channels=4", keys);
    ck_assert_ptr_ne(args, NULL);

    ck_assert_int_eq(pa_modargs_get_sample_spec(args, &value), 0);
    ck_assert_int_eq(value.rate, 1234);
    ck_assert_int_eq(value.format, PA_SAMPLE_S16BE);
    ck_assert_int_eq(value.channels, 4);

    pa_modargs_free(args);
}
END_TEST

// test pa_modargs_get_sample_spec for invalid rate detection
START_TEST (modargs_test_get_sample_spec_2) {
    const char* keys[] = {
        "rate",
        "format",
        "channels",
        NULL
    };
    pa_modargs *args;
    pa_sample_spec value;

    args = pa_modargs_new("rate=0 format=s16be channels=4", keys);
    ck_assert_ptr_ne(args, NULL);

    ck_assert_int_lt(pa_modargs_get_sample_spec(args, &value), 0);

    pa_modargs_free(args);
}
END_TEST

// test pa_modargs_get_sample_spec for invalid format detection
START_TEST (modargs_test_get_sample_spec_3) {
    const char* keys[] = {
        "rate",
        "format",
        "channels",
        NULL
    };
    pa_modargs *args;
    pa_sample_spec value;

    args = pa_modargs_new("rate=1234 format=whatever channels=4", keys);
    ck_assert_ptr_ne(args, NULL);

    ck_assert_int_lt(pa_modargs_get_sample_spec(args, &value), 0);

    pa_modargs_free(args);
}
END_TEST

// test pa_modargs_get_sample_spec for invalid channels detection
START_TEST (modargs_test_get_sample_spec_4) {
    const char* keys[] = {
        "rate",
        "format",
        "channels",
        NULL
    };
    pa_modargs *args;
    pa_sample_spec value;

    args = pa_modargs_new("rate=1234 format=s16be channels=0", keys);
    ck_assert_ptr_ne(args, NULL);

    ck_assert_int_lt(pa_modargs_get_sample_spec(args, &value), 0);

    pa_modargs_free(args);
}
END_TEST

// test pa_modargs_iterate()
START_TEST (modargs_test_modargs_iterate) {
    const char* keys[] = {
        "one",
        "two",
        "four",
        "eight",
        NULL
    };
    pa_modargs *args;
    void* state;
    char const* key;
    uint32_t sum;

    args = pa_modargs_new("one=1 two=2 eight=8", keys);
    ck_assert_ptr_ne(args, NULL);

    state = NULL;
    sum = 0;
    for (key = pa_modargs_iterate(args, &state); key != NULL; key = pa_modargs_iterate(args, &state)) {
        uint32_t tmp;
        ck_assert_ptr_ne(key, NULL);
        ck_assert_int_eq(pa_modargs_get_value_u32(args, key, &tmp), 0);
        sum += tmp;
    }
    ck_assert_int_eq(sum, 11);

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
    tcase_add_test(tc, modargs_test_get_value_u32);
    tcase_add_test(tc, modargs_test_get_value_s32);
    tcase_add_test(tc, modargs_test_get_value_boolean);
    tcase_add_test(tc, modargs_test_get_value_double);
    tcase_add_test(tc, modargs_test_get_value_volume);
    tcase_add_test(tc, modargs_test_get_sample_rate_1);
    tcase_add_test(tc, modargs_test_get_sample_rate_2);
    tcase_add_test(tc, modargs_test_get_sample_rate_3);
    tcase_add_test(tc, modargs_test_get_sample_spec_1);
    tcase_add_test(tc, modargs_test_get_sample_spec_2);
    tcase_add_test(tc, modargs_test_get_sample_spec_3);
    tcase_add_test(tc, modargs_test_get_sample_spec_4);
    tcase_add_test(tc, modargs_test_modargs_iterate);
    suite_add_tcase(s, tc);

    sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
