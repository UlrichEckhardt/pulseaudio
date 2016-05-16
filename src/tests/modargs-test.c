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

START_TEST (modargs_test_get_value_u32) {
    const char* keys[] = {
        "abc",
        "def",
        NULL
    };
    pa_modargs *args;
    uint32_t value;

    args = pa_modargs_new("abc=123 def=xyz", keys);
    ck_assert_ptr_ne(args, NULL);

    /* test extracting and parsing */
    ck_assert_int_eq(pa_modargs_get_value_u32(args, "abc", &value), 0);
    ck_assert_int_eq(value, 123);

    /* test extracting and parsing failure */
    ck_assert_int_lt(pa_modargs_get_value_u32(args, "def", &value), 0);

    pa_modargs_free(args);
}
END_TEST

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

    /* test extracting and parsing */
    ck_assert_int_eq(pa_modargs_get_value_s32(args, "abc", &value), 0);
    ck_assert_int_eq(value, -123);

    /* test extracting and parsing failure */
    ck_assert_int_lt(pa_modargs_get_value_s32(args, "def", &value), 0);

    pa_modargs_free(args);
}
END_TEST

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

    /* test extracting and parsing */
    ck_assert_int_eq(pa_modargs_get_value_boolean(args, "abc", &value), 0);
    ck_assert(value == true);

    /* test extracting and parsing failure */
    ck_assert_int_lt(pa_modargs_get_value_boolean(args, "def", &value), 0);

    pa_modargs_free(args);
}
END_TEST

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

    /* test extracting and parsing */
    ck_assert_int_eq(pa_modargs_get_value_double(args, "abc", &value), 0);
    ck_assert(value <= 12.26);
    ck_assert(value >= 12.24);

    /* test extracting and parsing failure */
    ck_assert_int_lt(pa_modargs_get_value_double(args, "def", &value), 0);

    pa_modargs_free(args);
}
END_TEST

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

    /* test extracting and parsing */
    ck_assert_int_eq(pa_modargs_get_value_volume(args, "valueA", &value), 0);
    ck_assert_int_eq(value, 104869);

    /* test extracting and parsing failure */
    ck_assert_int_lt(pa_modargs_get_value_volume(args, "valueZ", &value), 0);

    pa_modargs_free(args);
}
END_TEST

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
// /* Return channel map data from the argument "channel_map" if name is NULL, otherwise read from the specified argument */
// int pa_modargs_get_channel_map(pa_modargs *ma, const char *name, pa_channel_map *map);

// /* Return resample method from the argument "resample_method" */
// int pa_modargs_get_resample_method(pa_modargs *ma, pa_resample_method_t *method);

// /* Combination of pa_modargs_get_sample_spec() and
// pa_modargs_get_channel_map(). Not always suitable, since this routine
// initializes the map parameter based on the channels field of the ss
// structure if no channel_map is found, using pa_channel_map_init_auto() */

// int pa_modargs_get_sample_spec_and_channel_map(pa_modargs *ma, pa_sample_spec *ss, pa_channel_map *map, pa_channel_map_def_t def);

// /* Return alternate sample rate from "alternate_sample_rate" parameter */
// int pa_modargs_get_alternate_sample_rate(pa_modargs *ma, uint32_t *alternate_rate);

// int pa_modargs_get_proplist(pa_modargs *ma, const char *name, pa_proplist *p, pa_update_mode_t m);

// /* Iterate through the module argument list. The user should allocate a
//  * state variable of type void* and initialize it with NULL. A pointer
//  * to this variable should then be passed to pa_modargs_iterate()
//  * which should be called in a loop until it returns NULL which
//  * signifies EOL. On each invocation this function will return the
//  * key string for the next entry. The keys in the argument list do not
//  * have any particular order. */
// const char *pa_modargs_iterate(pa_modargs *ma, void **state);
