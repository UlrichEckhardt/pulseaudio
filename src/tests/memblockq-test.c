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

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include <check.h>

#include <pulsecore/memblockq.h>
#include <pulsecore/log.h>
#include <pulsecore/macro.h>
#include <pulsecore/strbuf.h>
#include <pulsecore/core-util.h>

#include <pulse/xmalloc.h>

static const char *fixed[] = {
    "1122444411441144__22__11______3333______________________________",
    "__________________3333__________________________________________"
};
static const char *manual[] = {
    "1122444411441144__22__11______3333______________________________",
    "__________________3333______________________________"
};

/*
 * utility function to create a memchunk
 */
static pa_memchunk memchunk_from_str(pa_mempool *p, const char* data)
{
    pa_memchunk res;
    size_t size = strlen(data);
    res.memblock = pa_memblock_new_fixed(p, (void*)data, size, true);
    ck_assert_ptr_ne(res.memblock, NULL);
    res.index = 0;
    res.length = pa_memblock_get_length(res.memblock);
    return res;
}

static void dump_chunk(const pa_memchunk *chunk, pa_strbuf *buf) {
    size_t n;
    void *q;
    char *e;

    fail_unless(chunk != NULL);

    q = pa_memblock_acquire(chunk->memblock);
    for (e = (char*) q + chunk->index, n = 0; n < chunk->length; n++, e++) {
        fprintf(stderr, "%c", *e);
        pa_strbuf_putc(buf, *e);
    }
    pa_memblock_release(chunk->memblock);
}

static void dump(pa_memblockq *bq, int n) {
    pa_memchunk out;
    pa_strbuf *buf;
    char *str;

    pa_assert(bq);

    /* First let's dump this as fixed block */
    fprintf(stderr, "FIXED >");
    pa_memblockq_peek_fixed_size(bq, 64, &out);
    buf = pa_strbuf_new();
    dump_chunk(&out, buf);
    pa_memblock_unref(out.memblock);
    str = pa_strbuf_to_string_free(buf);
    fail_unless(pa_streq(str, fixed[n]));
    pa_xfree(str);
    fprintf(stderr, "<\n");

    /* Then let's dump the queue manually */
    fprintf(stderr, "MANUAL>");

    buf = pa_strbuf_new();
    for (;;) {
        if (pa_memblockq_peek(bq, &out) < 0)
            break;

        dump_chunk(&out, buf);
        pa_memblock_unref(out.memblock);
        pa_memblockq_drop(bq, out.length);
    }
    str = pa_strbuf_to_string_free(buf);
    fail_unless(pa_streq(str, manual[n]));
    pa_xfree(str);
    fprintf(stderr, "<\n");
}

/*
 * utility function to validate invariants
 *
 * The different values like base, maxlength etc follow certain rules.
 * This convenience function makes sure that changes don't violate
 * these rules.
 */
static void check_queue_invariants(pa_memblockq *bq) {
    size_t base = pa_memblockq_get_base(bq);
    size_t maxlength = pa_memblockq_get_maxlength(bq);
    size_t tlength = pa_memblockq_get_tlength(bq);
    size_t minreq = pa_memblockq_get_minreq(bq);
    size_t prebuf = pa_memblockq_get_prebuf(bq);
    size_t length = pa_memblockq_get_length(bq);
    size_t missing = pa_memblockq_missing(bq);

    /* base > zero */
    ck_assert_int_gt(base, 0);

    /* maxlength multiple of base
     * maxlength >= base */
    ck_assert_int_eq(maxlength % base, 0);
    ck_assert_int_ge(maxlength, base);

    /* tlength multiple of base
     * tlength >= base
     * tlength <= maxlength */
    ck_assert_int_eq(tlength % base, 0);
    ck_assert_int_ge(tlength, base);
    ck_assert_int_le(tlength, maxlength);

    /* minreq multiple of base
     * minreq >= base
     * minreq <= tlength */
    ck_assert_int_eq(minreq % base, 0);
    ck_assert_int_ge(minreq, base);
    ck_assert_int_le(minreq, tlength);

    /* prebuf multiple of base
     * prebuf >= 0
     * prebuf <= tlength + base - minreq
     * prebuf <= tlength (because minreq >= base) */
    ck_assert_int_eq(prebuf % base, 0);
    ck_assert_int_ge(prebuf, 0);
    ck_assert_int_le(prebuf, tlength + base - minreq);
    ck_assert_int_le(prebuf, tlength);

    /* length >= 0
     * length <= maxlength */
    ck_assert_int_ge(length, 0);
    ck_assert_int_le(length, maxlength);

    /* missing >= 0
     * missing <= tlength
     * minimum reported amount of missing data is minreq
     * reported amount of missing data is target length minus actual length */
    ck_assert_int_ge(missing, 0);
    ck_assert_int_le(missing, tlength);
    ck_assert((missing == 0) || (missing >= minreq));
    ck_assert((missing == 0) || (missing == tlength - length));
}

START_TEST (memchunk_from_str_test) {
    pa_mempool *p;
    pa_memchunk chunk;

    p = pa_mempool_new(PA_MEM_TYPE_PRIVATE, 0, true);
    ck_assert_ptr_ne(p, NULL);

    /* allocate memchunk and check default settings */
    chunk = memchunk_from_str(p, "abcd");
    ck_assert_ptr_ne(chunk.memblock, NULL);
    ck_assert_int_eq(chunk.index, 0);
    ck_assert_int_eq(chunk.length, 4);

    /* cleanup */
    pa_memblock_unref(chunk.memblock);
    pa_mempool_unref(p);
}
END_TEST

START_TEST (memblockq_test_initial_properties) {
    pa_mempool *p;
    pa_memblockq *bq;
    pa_memchunk silence;
    pa_sample_spec ss = {
        .format = PA_SAMPLE_S32BE,
        .rate = 48000,
        .channels = 1
    };
    int64_t idx = 0;
    size_t maxlength = 100;
    size_t tlength = 20;
    size_t prebuf = 16;
    size_t minreq = 8;
    size_t maxrewind = 40;

    p = pa_mempool_new(PA_MEM_TYPE_PRIVATE, 0, true);
    ck_assert_ptr_ne(p, NULL);

    silence = memchunk_from_str(p, "__");

    bq = pa_memblockq_new("test memblockq", idx, maxlength, tlength, &ss, prebuf, minreq, maxrewind, &silence);
    fail_unless(bq != NULL);

    /* check initial properties */
    ck_assert_int_eq(pa_memblockq_is_readable(bq), false);
    ck_assert_int_eq(pa_memblockq_get_length(bq), 0);
    ck_assert_int_eq(pa_memblockq_missing(bq), tlength);
    ck_assert_int_eq(pa_memblockq_get_maxlength(bq), maxlength);
    ck_assert_int_eq(pa_memblockq_get_tlength(bq), tlength);
    ck_assert_int_eq(pa_memblockq_get_prebuf(bq), prebuf);
    ck_assert_int_eq(pa_memblockq_get_minreq(bq), minreq);
    ck_assert_int_eq(pa_memblockq_get_maxrewind(bq), maxrewind);
    ck_assert_int_eq(pa_memblockq_get_base(bq), pa_frame_size(&ss));
    ck_assert_int_eq(pa_memblockq_get_read_index(bq), 0);
    ck_assert_int_eq(pa_memblockq_get_write_index(bq), 0);

    check_queue_invariants(bq);

    /* Check reporting of missing bytes:
     * Initially, tlength bytes are missing. The second call doesn't
     * report additional missing data since the first call. */
    ck_assert_int_eq(pa_memblockq_pop_missing(bq), tlength);
    ck_assert_int_eq(pa_memblockq_pop_missing(bq), 0);

    /* cleanup */
    pa_memblockq_free(bq);
    pa_memblock_unref(silence.memblock);
    pa_mempool_unref(p);
}
END_TEST

START_TEST (memblockq_test) {
    int ret;

    pa_mempool *p;
    pa_memblockq *bq;
    pa_memchunk chunk1, chunk2, chunk3, chunk4;
    pa_memchunk silence;
    pa_sample_spec ss = {
        .format = PA_SAMPLE_S16LE,
        .rate = 48000,
        .channels = 1
    };

    p = pa_mempool_new(PA_MEM_TYPE_PRIVATE, 0, true);
    ck_assert_ptr_ne(p, NULL);

    silence = memchunk_from_str(p, "__");

    bq = pa_memblockq_new("test memblockq", 0, 200, 10, &ss, 4, 4, 40, &silence);
    fail_unless(bq != NULL);
    check_queue_invariants(bq);

    chunk1 = memchunk_from_str(p, "11");
    chunk2 = memchunk_from_str(p, "XX22");
    chunk2.index += 2;
    chunk2.length -= 2;
    chunk3 = memchunk_from_str(p, "3333");
    chunk4 = memchunk_from_str(p, "44444444");

    ret = pa_memblockq_push(bq, &chunk1);
    fail_unless(ret == 0);

    ret = pa_memblockq_push(bq, &chunk2);
    fail_unless(ret == 0);

    ret = pa_memblockq_push(bq, &chunk3);
    fail_unless(ret == 0);

    ret = pa_memblockq_push(bq, &chunk4);
    fail_unless(ret == 0);

    check_queue_invariants(bq);

    pa_memblockq_seek(bq, -6, 0, true);
    ret = pa_memblockq_push(bq, &chunk3);
    fail_unless(ret == 0);

    pa_memblockq_seek(bq, -2, 0, true);
    ret = pa_memblockq_push(bq, &chunk1);
    fail_unless(ret == 0);

    pa_memblockq_seek(bq, -10, 0, true);
    ret = pa_memblockq_push(bq, &chunk4);
    fail_unless(ret == 0);

    pa_memblockq_seek(bq, 10, 0, true);

    ret = pa_memblockq_push(bq, &chunk1);
    fail_unless(ret == 0);

    pa_memblockq_seek(bq, -6, 0, true);
    ret = pa_memblockq_push(bq, &chunk2);
    fail_unless(ret == 0);

    /* Test splitting */
    pa_memblockq_seek(bq, -12, 0, true);
    ret = pa_memblockq_push(bq, &chunk1);
    fail_unless(ret == 0);

    pa_memblockq_seek(bq, 20, 0, true);

    /* Test merging */
    ret = pa_memblockq_push(bq, &chunk3);
    fail_unless(ret == 0);
    pa_memblockq_seek(bq, -2, 0, true);

    chunk3.index += 2;
    chunk3.length -= 2;
    ret = pa_memblockq_push(bq, &chunk3);
    fail_unless(ret == 0);

    pa_memblockq_seek(bq, 30, PA_SEEK_RELATIVE, true);

    dump(bq, 0);

    pa_memblockq_rewind(bq, 52);

    dump(bq, 1);

    check_queue_invariants(bq);

    pa_memblockq_free(bq);
    pa_memblock_unref(silence.memblock);
    pa_memblock_unref(chunk1.memblock);
    pa_memblock_unref(chunk2.memblock);
    pa_memblock_unref(chunk3.memblock);
    pa_memblock_unref(chunk4.memblock);

    pa_mempool_unref(p);
}
END_TEST

START_TEST (memblockq_test_length_changes) {
    pa_mempool *p;
    pa_memblockq *bq;
    pa_memchunk silence, data;
    pa_sample_spec ss = {
        .format = PA_SAMPLE_S32BE,
        .rate = 48000,
        .channels = 1
    };
    int64_t idx = 0;
    size_t maxlength = 60;
    size_t tlength = 40;
    size_t prebuf = 16;
    size_t minreq = 20;
    size_t maxrewind = 40;

    p = pa_mempool_new(PA_MEM_TYPE_PRIVATE, 0, true);
    ck_assert_ptr_ne(p, NULL);

    silence = memchunk_from_str(p, "____");

    bq = pa_memblockq_new("test memblockq", idx, maxlength, tlength, &ss, prebuf, minreq, maxrewind, &silence);
    fail_unless(bq != NULL);

    data = memchunk_from_str(p, "12345678");

    /* insert some data */
    ck_assert_int_eq(pa_memblockq_push(bq, &data), 0);
    ck_assert_int_eq(pa_memblockq_push(bq, &data), 0);
    ck_assert_int_eq(pa_memblockq_push(bq, &data), 0);
    ck_assert_int_eq(pa_memblockq_push(bq, &data), 0);

    /* check state */
    ck_assert_int_eq(pa_memblockq_get_length(bq), 32);
    ck_assert_int_eq(pa_memblockq_missing(bq), 0);

    /* adjust maximum length
       This might modify tlength, prebuf, minreq, too. */
    pa_memblockq_set_maxlength(bq, maxlength/2);
    check_queue_invariants(bq);

    /* adjust target length
       This might modify minreq, too. */
    pa_memblockq_set_tlength(bq, tlength/2);
    check_queue_invariants(bq);

    /* adjust minimum requested length
       This might modify prebuf, too. */
    pa_memblockq_set_minreq(bq, minreq/2);
    check_queue_invariants(bq);

    /* adjust prebuffer length */
    pa_memblockq_set_prebuf(bq, prebuf/2);
    check_queue_invariants(bq);

    /* cleanup */
    pa_memblockq_free(bq);
    pa_memblock_unref(silence.memblock);
    pa_memblock_unref(data.memblock);
    pa_mempool_unref(p);
}
END_TEST

START_TEST (memblockq_test_pop_missing) {
    pa_mempool *p;
    pa_memblockq *bq;
    pa_memchunk silence, data, chunk;
    pa_sample_spec ss = {
        .format = PA_SAMPLE_S16BE,
        .rate = 48000,
        .channels = 1
    };
    int64_t idx = 0;
    size_t maxlength = 200;
    size_t tlength = 100;
    size_t prebuf = 0;
    size_t minreq = 80;
    size_t maxrewind = 0;

    p = pa_mempool_new(PA_MEM_TYPE_PRIVATE, 0, true);
    ck_assert_ptr_ne(p, NULL);

    silence = memchunk_from_str(p, "____");
    data = memchunk_from_str(p, "1234567890");

    bq = pa_memblockq_new("test memblockq", idx, maxlength, tlength, &ss, prebuf, minreq, maxrewind, &silence);
    fail_unless(bq != NULL);

    /* initially, the whole target length of bytes is missing */
    ck_assert_int_eq(pa_memblockq_missing(bq), tlength);
    ck_assert_int_eq(pa_memblockq_pop_missing(bq), tlength);

    /* add 20 bytes of data */
    for (int i=0; i!=2; ++i)
        ck_assert_int_eq(pa_memblockq_push(bq, &data), 0);
    check_queue_invariants(bq);

    /* the missing bytes are reduced, but no new missing data is reported */
    ck_assert_int_eq(pa_memblockq_missing(bq), tlength - 20);
    ck_assert_int_eq(pa_memblockq_pop_missing(bq), 0);

    /* fill up to 100 bytes of data */
    for (int i=0; i!=8; ++i)
        ck_assert_int_eq(pa_memblockq_push(bq, &data), 0);
    check_queue_invariants(bq);

    /* queue fill level is at target level now */
    ck_assert_int_eq(pa_memblockq_missing(bq), 0);
    ck_assert_int_eq(pa_memblockq_pop_missing(bq), 0);

    /* pop 40 bytes of data, down to 60 bytes fill level */
    ck_assert_int_eq(pa_memblockq_peek_fixed_size(bq, 40, &chunk), 0);
    pa_memblockq_drop(bq, 40);
    ck_assert_int_eq(chunk.length - chunk.index, 40);
    pa_memblock_unref(chunk.memblock);
    check_queue_invariants(bq);

    /* queue fill level is 40 bytes under target length
       This is less than minreq, so no missing data is reported statically,
       but dynamically via pop_missing(), it reports 40 bytes. */
    ck_assert_int_eq(pa_memblockq_missing(bq), 0);
    ck_assert_int_eq(pa_memblockq_pop_missing(bq), 40);

    /* add 30 bytes of data, up to 90 bytes fill level */
    for (int i=0; i!=3; ++i)
        ck_assert_int_eq(pa_memblockq_push(bq, &data), 0);
    check_queue_invariants(bq);

    /* queue fill level is 10 bytes under target length
       This is less than minreq, so no missing data is reported. */
    ck_assert_int_eq(pa_memblockq_missing(bq), 0);
    ck_assert_int_eq(pa_memblockq_pop_missing(bq), 0);

    /* pop 20 bytes of data, down to 70 bytes of data */
    ck_assert_int_eq(pa_memblockq_peek_fixed_size(bq, 20, &chunk), 0);
    pa_memblockq_drop(bq, 20);
    ck_assert_int_eq(chunk.length - chunk.index, 20);
    pa_memblock_unref(chunk.memblock);
    check_queue_invariants(bq);

    /* queue fill level is 30 bytes under target length
       This is less than minreq, so no missing data is reported statically,
       but dynamically via pop_missing(), it reports 20 bytes. */
    ck_assert_int_eq(pa_memblockq_missing(bq), 0);
    ck_assert_int_eq(pa_memblockq_pop_missing(bq), 20);

    /* add 50 bytes of data, up to 120 bytes fill level */
    for (int i=0; i!=5; ++i)
        ck_assert_int_eq(pa_memblockq_push(bq, &data), 0);
    check_queue_invariants(bq);

    /* queue fill level is above target level, so no missing data is reported. */
    ck_assert_int_eq(pa_memblockq_missing(bq), 0);
    ck_assert_int_eq(pa_memblockq_pop_missing(bq), 0);

    /* pop 20 bytes of data, down the target level */
    ck_assert_int_eq(pa_memblockq_peek_fixed_size(bq, 20, &chunk), 0);
    pa_memblockq_drop(bq, 20);
    ck_assert_int_eq(chunk.length - chunk.index, 20);
    pa_memblock_unref(chunk.memblock);
    check_queue_invariants(bq);

    /* queue fill level is at target level now
       No statically or dynamically missing data should be reported. */
    ck_assert_int_eq(pa_memblockq_missing(bq), 0);
    ck_assert_int_eq(pa_memblockq_pop_missing(bq), 0);

    /* cleanup */
    pa_memblockq_free(bq);
    pa_memblock_unref(silence.memblock);
    pa_memblock_unref(data.memblock);
    pa_mempool_unref(p);
}
END_TEST

int main(int argc, char *argv[]) {
    int failed = 0;
    Suite *s;
    TCase *tc;
    SRunner *sr;

    if (!getenv("MAKE_CHECK"))
        pa_log_set_level(PA_LOG_DEBUG);

    s = suite_create("Memblock Queue");
    tc = tcase_create("memblockq");
    tcase_add_test(tc, memchunk_from_str_test);
    tcase_add_test(tc, memblockq_test_initial_properties);
    tcase_add_test(tc, memblockq_test);
    tcase_add_test(tc, memblockq_test_length_changes);
    tcase_add_test(tc, memblockq_test_pop_missing);
    suite_add_tcase(s, tc);

    sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
