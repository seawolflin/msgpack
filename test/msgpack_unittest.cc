/*
 *  Copyright (c) 2016 - 2017  seawolflin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "ytest.h"

#include "msgpack.h"

static uint8_t test_buf[1024];
static msgpack_buffer_t test_mbuf;
static msgpack_unpacker_t test_unpacker;
static uint8_t test_type;
static char test_data[1024];
static uint32_t test_data_len;
#define TEST_INIT_MBUF(mb, b, l) init_msgpack_buffer(mb, b, l)
#define TEST_INIT_UPR(upr, b, l, used) init_msgpack_unpacker(upr, b, l, used)

#define TEST_CHECK_EQUAL(e, a, len) \
    do { \
        size_t i = 0; \
        const uint8_t *expect = (const uint8_t *)e; \
        const uint8_t *actual = (const uint8_t *)a; \
        while(i != len) { \
            if (expect[i] != actual[i]) { \
                EXPECT_EQ(expect[i], actual[i]); \
                break; \
            } \
            i++; \
        } \
    } while (0)

        // EXPECT_EQ(0, memcmp(expt, buf, expt_len)); \

#define TEST_MSGPACK_WRITE_CHECK(expt, expt_len, op) \
    do { \
        TEST_INIT_MBUF(&test_mbuf, test_buf, 1024); \
        EXPECT_TRUE(op); \
        TEST_CHECK_EQUAL((expt), test_buf, (expt_len)); \
        EXPECT_EQ((expt_len), test_mbuf.len); \
        EXPECT_EQ(MSGPACK_EOK, msgpack_errno()); \
    } while(0)

#define TEST_MSGPACK_WSTR_CHECK(expt, check_len, expt_len, op) \
    do { \
        TEST_INIT_MBUF(&test_mbuf, test_buf, 1024); \
        EXPECT_TRUE(op); \
        TEST_CHECK_EQUAL(expt, test_buf, (check_len)); \
        EXPECT_EQ(expt_len, test_mbuf.len); \
        EXPECT_EQ(MSGPACK_EOK, msgpack_errno()); \
    } while(0)

#define TEST_MSGPACK_READ_CHECK(in, in_len, type, expt_type, data_len, expt_data_len, expt_data, T, op) \
    do { \
        memcpy(test_buf, in, in_len); \
        TEST_INIT_UPR(&test_unpacker, test_buf, in_len, 0); \
        test_type = type; \
        test_data_len = data_len; \
        EXPECT_TRUE(op); \
        EXPECT_EQ(expt_type, test_type); \
        EXPECT_EQ(expt_data_len, test_data_len); \
        if ((expt_data_len) != 0) { \
            EXPECT_EQ(expt_data, *((T *)test_data)); \
        } \
        EXPECT_EQ(in_len, test_unpacker.pos); \
        EXPECT_EQ(MSGPACK_EOK, msgpack_errno()); \
    } while (0)

#define TEST_MSGPACK_RSTR_CHECK(in, in_len, buf_len, type, expt_type, expt_data_len, expt_data, check_len, op) \
    do { \
        memcpy(test_buf, in, in_len); \
        TEST_INIT_UPR(&test_unpacker, test_buf, buf_len, 0); \
        test_type = type; \
        test_data_len = 1024; \
        EXPECT_TRUE(op); \
        EXPECT_EQ(expt_type, test_type); \
        EXPECT_EQ(expt_data_len, test_data_len); \
        TEST_CHECK_EQUAL(expt_data, test_data, check_len); \
        EXPECT_EQ(buf_len, test_unpacker.pos); \
        EXPECT_EQ(MSGPACK_EOK, msgpack_errno()); \
    } while (0)

#define TEST_MSGPACK_READ_CHECK_LEN(in, in_len, type, expt_type, expt_data_len, op) \
    do { \
        memcpy(test_buf, in, in_len); \
        TEST_INIT_UPR(&test_unpacker, test_buf, in_len, 0); \
        test_type = type; \
        EXPECT_TRUE(op); \
        EXPECT_EQ(expt_type, test_type); \
        EXPECT_EQ(expt_data_len, test_data_len); \
        EXPECT_EQ(0, test_unpacker.pos); \
        EXPECT_EQ(MSGPACK_EOK, msgpack_errno()); \
    } while (0)

#define TEST_MSGPACK_WRITE_CHECK_ERROR(expt, mb, b, l, op) \
    do { \
        TEST_INIT_MBUF((mb), (b), (l)); \
        EXPECT_FALSE(op); \
        if (mb) { \
            EXPECT_EQ(0, (mb)->len); \
        } \
        EXPECT_EQ((expt), msgpack_errno()); \
    } while (0)

#define TEST_MSGPACK_READ_CHECK_ERROR(expt, upr, b, l, used, op, other_cmd) \
    do { \
        TEST_INIT_UPR(upr, b, l, used); \
        other_cmd; \
        EXPECT_FALSE(op); \
        if (upr) { \
            EXPECT_EQ(used, (upr)->pos); \
        } \
        EXPECT_EQ(expt, msgpack_errno()); \
    } while (0)

TEST(msgpack, write_simple) {
    TEST_MSGPACK_WRITE_CHECK("\xc0", 1, msgpack_write_nil(&test_mbuf));
    TEST_MSGPACK_WRITE_CHECK("\xc3", 1, msgpack_write_bool(&test_mbuf, true));
    TEST_MSGPACK_WRITE_CHECK("\xc2", 1, msgpack_write_bool(&test_mbuf, false));
    TEST_MSGPACK_WRITE_CHECK("\xc3", 1, msgpack_write_true(&test_mbuf));
    TEST_MSGPACK_WRITE_CHECK("\xc2", 1, msgpack_write_false(&test_mbuf));
    TEST_MSGPACK_WRITE_CHECK("\xe0", 1, msgpack_write_smallint(&test_mbuf, -32));
    TEST_MSGPACK_WRITE_CHECK("\x7f", 1, msgpack_write_smallint(&test_mbuf, 127));
    TEST_MSGPACK_WRITE_CHECK("\x00", 1, msgpack_write_smallint(&test_mbuf, 0));
    TEST_MSGPACK_WRITE_CHECK("\xcc\x80", 2, msgpack_write_u8(&test_mbuf, 0x80));
    TEST_MSGPACK_WRITE_CHECK("\xcc\x00", 2, msgpack_write_u8(&test_mbuf, 0));
    TEST_MSGPACK_WRITE_CHECK("\xcd\xa5\x5a", 3, msgpack_write_u16(&test_mbuf, 0xa55a));
    TEST_MSGPACK_WRITE_CHECK("\xcd\x00\xa5", 3, msgpack_write_u16(&test_mbuf, 0xa5));
    TEST_MSGPACK_WRITE_CHECK("\xce\xa5\x5a\x55\xaa", 5, msgpack_write_u32(&test_mbuf, 0xa55a55aa));
    TEST_MSGPACK_WRITE_CHECK("\xce\x00\x00\x00\xa5", 5, msgpack_write_u32(&test_mbuf, 0xa5));
    TEST_MSGPACK_WRITE_CHECK("\xcf\xa5\x5a\x55\xaa\xbb\xcc\xdd\xee", 9, msgpack_write_u64(&test_mbuf, 0xa55a55aabbccddee));
    TEST_MSGPACK_WRITE_CHECK("\xcf\x00\x00\x00\x00\x00\x00\x00\xa5", 9, msgpack_write_u64(&test_mbuf, 0xa5));
    TEST_MSGPACK_WRITE_CHECK("\xd0\xa6", 2, msgpack_write_s8(&test_mbuf, -0x5a));
    TEST_MSGPACK_WRITE_CHECK("\xd0\x01", 2, msgpack_write_s8(&test_mbuf, 0x01));
    TEST_MSGPACK_WRITE_CHECK("\xd1\xaa\xa6", 3, msgpack_write_s16(&test_mbuf, -0x555a));
    TEST_MSGPACK_WRITE_CHECK("\xd1\xff\x5b", 3, msgpack_write_s16(&test_mbuf, -0xa5));
    TEST_MSGPACK_WRITE_CHECK("\xd2\xaa\xa5\xaa\x56", 5, msgpack_write_s32(&test_mbuf, -0x555a55aa));
    TEST_MSGPACK_WRITE_CHECK("\xd2\xff\xff\xff\x5b", 5, msgpack_write_s32(&test_mbuf, -0xa5));
    TEST_MSGPACK_WRITE_CHECK("\xd3\xaa\xa5\xaa\x55\x44\x33\x22\x12", 9, msgpack_write_s64(&test_mbuf, -0x555a55aabbccddee));
    TEST_MSGPACK_WRITE_CHECK("\xd3\xff\xff\xff\xff\xff\xff\xff\x5b", 9, msgpack_write_s64(&test_mbuf, -0xa5));
    TEST_MSGPACK_WRITE_CHECK("\xe0", 1, msgpack_write_integer(&test_mbuf, -32));
    TEST_MSGPACK_WRITE_CHECK("\xe1", 1, msgpack_write_integer(&test_mbuf, -31));
    TEST_MSGPACK_WRITE_CHECK("\x7f", 1, msgpack_write_integer(&test_mbuf, 127));
    TEST_MSGPACK_WRITE_CHECK("\x01", 1, msgpack_write_integer(&test_mbuf, 1));
    TEST_MSGPACK_WRITE_CHECK("\x00", 1, msgpack_write_integer(&test_mbuf, 0));
    TEST_MSGPACK_WRITE_CHECK("\xcc\x80", 2, msgpack_write_integer(&test_mbuf, 0x80));
    TEST_MSGPACK_WRITE_CHECK("\xcd\xa5\x5a", 3, msgpack_write_integer(&test_mbuf, 0xa55a));
    TEST_MSGPACK_WRITE_CHECK("\xce\xa5\x5a\x55\xaa", 5, msgpack_write_integer(&test_mbuf, 0xa55a55aa));
    TEST_MSGPACK_WRITE_CHECK("\xcf\xa5\x5a\x55\xaa\xbb\xcc\xdd\xee", 9, msgpack_write_integer(&test_mbuf, 0xa55a55aabbccddee));
    TEST_MSGPACK_WRITE_CHECK("\xd0\xa6", 2, msgpack_write_integer(&test_mbuf, -0x5a));
    TEST_MSGPACK_WRITE_CHECK("\xd1\xaa\xa6", 3, msgpack_write_integer(&test_mbuf, -0x555a));
    TEST_MSGPACK_WRITE_CHECK("\xd2\xaa\xa5\xaa\x56", 5, msgpack_write_integer(&test_mbuf, -0x555a55aa));
    TEST_MSGPACK_WRITE_CHECK("\xd3\xaa\xa5\xaa\x55\x44\x33\x22\x12", 9, msgpack_write_integer(&test_mbuf, -0x555a55aabbccddee));

    TEST_MSGPACK_WRITE_CHECK("\xca\x40\x49\x0f\xdb", 5, msgpack_write_float(&test_mbuf, 3.1415927));
    TEST_MSGPACK_WRITE_CHECK("\xcb\x40\x09\x21\xfb\x5a\x7e\xd1\x97", 9, msgpack_write_double(&test_mbuf, 3.1415927));
    TEST_MSGPACK_WRITE_CHECK("\xca\x00\x00\x00\x00", 5, msgpack_write_float(&test_mbuf, 0.0f));
    TEST_MSGPACK_WRITE_CHECK("\xca\x40\x2d\xf3\xb6", 5, msgpack_write_float(&test_mbuf, 2.718f));
    TEST_MSGPACK_WRITE_CHECK("\xca\xc0\x2d\xf3\xb6", 5, msgpack_write_float(&test_mbuf, -2.718f));
    TEST_MSGPACK_WRITE_CHECK("\xcb\x00\x00\x00\x00\x00\x00\x00\x00", 9, msgpack_write_double(&test_mbuf, 0.0));
    TEST_MSGPACK_WRITE_CHECK("\xcb\x40\x09\x21\xfb\x53\xc8\xd4\xf1", 9, msgpack_write_double(&test_mbuf, 3.14159265));
    TEST_MSGPACK_WRITE_CHECK("\xcb\xc0\x09\x21\xfb\x53\xc8\xd4\xf1", 9, msgpack_write_double(&test_mbuf, -3.14159265));
}

TEST(msgpack, write_string) {
    char str[] = "test";
    TEST_MSGPACK_WSTR_CHECK("\xa0", 1, 1, msgpack_write_smallstr(&test_mbuf, "", 0));
    TEST_MSGPACK_WSTR_CHECK("\xa1t", 2, 2, msgpack_write_smallstr(&test_mbuf, str, 1));
    TEST_MSGPACK_WSTR_CHECK("\xbftest", 6, 32, msgpack_write_smallstr(&test_mbuf, str, 31));
    TEST_MSGPACK_WSTR_CHECK("\xd9\xa5test", 7, 0xa7, msgpack_write_str8(&test_mbuf, str, 0xa5));
    TEST_MSGPACK_WSTR_CHECK("\xd9\xfftest", 7, 0x101, msgpack_write_str8(&test_mbuf, str, 0xff));
    TEST_MSGPACK_WSTR_CHECK("\xda\x00\xa5test", 8, 0xa8, msgpack_write_str16(&test_mbuf, str, 0xa5));
    TEST_MSGPACK_WSTR_CHECK("\xda\x02\x00test", 8, 0x203, msgpack_write_str16(&test_mbuf, str, 0x200));
    TEST_MSGPACK_WSTR_CHECK("\xdb\x00\x00\x00\xa5test", 10, 0xaa, msgpack_write_str32(&test_mbuf, str, 0xa5));
    TEST_MSGPACK_WSTR_CHECK("\xdb\x00\x00\x02\xa5test", 10, 0x2aa, msgpack_write_str32(&test_mbuf, str, 0x2a5));
    TEST_MSGPACK_WSTR_CHECK("\xa0", 1, 1, msgpack_write_str(&test_mbuf, "", 0));
    TEST_MSGPACK_WSTR_CHECK("\xa1t", 2, 2, msgpack_write_str(&test_mbuf, str, 1));
    TEST_MSGPACK_WSTR_CHECK("\xbftest", 6, 32, msgpack_write_str(&test_mbuf, str, 31));
    TEST_MSGPACK_WSTR_CHECK("\xd9\xa5test", 7, 0xa7, msgpack_write_str(&test_mbuf, str, 0xa5));
    TEST_MSGPACK_WSTR_CHECK("\xd9\xfftest", 7, 0x101, msgpack_write_str(&test_mbuf, str, 0xff));
    TEST_MSGPACK_WSTR_CHECK("\xda\x02\x00test", 8, 0x203, msgpack_write_str(&test_mbuf, str, 0x200));

    TEST_MSGPACK_WSTR_CHECK("\xc4\x00", 2, 2, msgpack_write_bin8(&test_mbuf, "", 0));
    TEST_MSGPACK_WSTR_CHECK("\xc4\x01t", 3, 3, msgpack_write_bin8(&test_mbuf, str, 1));
    TEST_MSGPACK_WSTR_CHECK("\xc4\x1ftest", 7, 33, msgpack_write_bin8(&test_mbuf, str, 31));
    TEST_MSGPACK_WSTR_CHECK("\xc4\xa5test", 7, 0xa7, msgpack_write_bin8(&test_mbuf, str, 0xa5));
    TEST_MSGPACK_WSTR_CHECK("\xc4\xfftest", 7, 0x101, msgpack_write_bin8(&test_mbuf, str, 0xff));
    TEST_MSGPACK_WSTR_CHECK("\xc5\x00\xa5test", 8, 0xa8, msgpack_write_bin16(&test_mbuf, str, 0xa5));
    TEST_MSGPACK_WSTR_CHECK("\xc5\x02\x00test", 8, 0x203, msgpack_write_bin16(&test_mbuf, str, 0x200));
    TEST_MSGPACK_WSTR_CHECK("\xc6\x00\x00\x00\xa5test", 10, 0xaa, msgpack_write_bin32(&test_mbuf, str, 0xa5));
    TEST_MSGPACK_WSTR_CHECK("\xc6\x00\x00\x02\xa5test", 10, 0x2aa, msgpack_write_bin32(&test_mbuf, str, 0x2a5));
    TEST_MSGPACK_WSTR_CHECK("\xc4\x00", 2, 2, msgpack_write_bin(&test_mbuf, "", 0));
    TEST_MSGPACK_WSTR_CHECK("\xc4\x01t", 3, 3, msgpack_write_bin(&test_mbuf, str, 1));
    TEST_MSGPACK_WSTR_CHECK("\xc4\x1ftest", 7, 33, msgpack_write_bin(&test_mbuf, str, 31));
    TEST_MSGPACK_WSTR_CHECK("\xc4\xa5test", 7, 0xa7, msgpack_write_bin(&test_mbuf, str, 0xa5));
    TEST_MSGPACK_WSTR_CHECK("\xc4\xfftest", 7, 0x101, msgpack_write_bin(&test_mbuf, str, 0xff));
    TEST_MSGPACK_WSTR_CHECK("\xc5\x02\x00test", 8, 0x203, msgpack_write_bin(&test_mbuf, str, 0x200));
}

TEST(msgpack, read_simple) {
    TEST_MSGPACK_READ_CHECK("\xc0", 1, MSGPACK_TYPE_ANY, MSGPACK_TYPE_NIL, 0, 0, NULL, void*, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xc0", 1, MSGPACK_TYPE_ANY, MSGPACK_TYPE_NIL, 0, 0, NULL, void*, msgpack_unpack(&test_unpacker, NULL, NULL, &test_type));
    TEST_MSGPACK_READ_CHECK("\xc0", 1, MSGPACK_TYPE_NIL, MSGPACK_TYPE_NIL, 0, 0, NULL, void*, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xc0", 1, MSGPACK_TYPE_ANY, MSGPACK_TYPE_NIL, 0, 0, NULL, void*, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xc3", 1, MSGPACK_TYPE_ANY, MSGPACK_TYPE_BOOL, 1, 1, true, bool, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xc2", 1, MSGPACK_TYPE_ANY, MSGPACK_TYPE_BOOL, 1, 1, false, bool, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xe0", 1, MSGPACK_TYPE_ANY, MSGPACK_TYPE_S8, 1, 1, -32, int8_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\x7f", 1, MSGPACK_TYPE_ANY, MSGPACK_TYPE_U8, 1, 1, 127, uint8_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\x00", 1, MSGPACK_TYPE_ANY, MSGPACK_TYPE_U8, 1, 1, 0, uint8_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xcc\x80", 2, MSGPACK_TYPE_ANY, MSGPACK_TYPE_U8, 1, 1, 0x80, uint8_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xcc\x00", 2, MSGPACK_TYPE_ANY, MSGPACK_TYPE_U8, 1, 1, 0, uint8_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xcd\xa5\x5a", 3, MSGPACK_TYPE_ANY, MSGPACK_TYPE_U16, 2, 2, 0xa55a, uint16_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xcd\x00\xa5", 3, MSGPACK_TYPE_ANY, MSGPACK_TYPE_U16, 2, 2, 0xa5, uint16_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xce\xa5\x5a\x55\xaa", 5, MSGPACK_TYPE_ANY, MSGPACK_TYPE_U32, 4, 4, 0xa55a55aa, uint32_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xce\x00\x00\x00\xa5", 5, MSGPACK_TYPE_ANY, MSGPACK_TYPE_U32, 4, 4, 0xa5, uint32_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xcf\xa5\x5a\x55\xaa\xbb\xcc\xdd\xee", 9, MSGPACK_TYPE_ANY, MSGPACK_TYPE_U64, 8, 8, 0xa55a55aabbccddee, uint64_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xcf\x00\x00\x00\x00\x00\x00\x00\xa5", 9, MSGPACK_TYPE_ANY, MSGPACK_TYPE_U64, 8, 8, 0xa5, uint64_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xd0\xa6", 2, MSGPACK_TYPE_ANY, MSGPACK_TYPE_S8, 1, 1, -0x5a, int8_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xd0\x01", 2, MSGPACK_TYPE_ANY, MSGPACK_TYPE_S8, 1, 1, 0x01, int8_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xd1\xaa\xa6", 3, MSGPACK_TYPE_ANY, MSGPACK_TYPE_S16, 2, 2, -0x555a, int16_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xd1\xff\x5b", 3, MSGPACK_TYPE_ANY, MSGPACK_TYPE_S16, 2, 2, -0xa5, int16_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xd2\xaa\xa5\xaa\x56", 5, MSGPACK_TYPE_ANY, MSGPACK_TYPE_S32, 4, 4, -0x555a55aa, int32_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xd2\xff\xff\xff\x5b", 5, MSGPACK_TYPE_ANY, MSGPACK_TYPE_S32, 4, 4, -0xa5, int32_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xd3\xaa\xa5\xaa\x55\x44\x33\x22\x12", 9, MSGPACK_TYPE_ANY, MSGPACK_TYPE_S64, 8, 8, -0x555a55aabbccddee, int64_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xd3\xff\xff\xff\xff\xff\xff\xff\x5b", 9, MSGPACK_TYPE_ANY, MSGPACK_TYPE_S64, 8, 8, -0xa5, int64_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xca\x40\x49\x0f\xdb", 5, MSGPACK_TYPE_ANY, MSGPACK_TYPE_SINGLE, 4, 4, 3.1415927f, float, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xcb\x40\x09\x21\xfb\x5a\x7e\xd1\x97", 9, MSGPACK_TYPE_ANY, MSGPACK_TYPE_DOUBLE, 8, 8, 3.1415927, double, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xca\x00\x00\x00\x00", 5, MSGPACK_TYPE_ANY, MSGPACK_TYPE_SINGLE, 4, 4, 0.0f, float, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xca\x40\x2d\xf3\xb6", 5, MSGPACK_TYPE_ANY, MSGPACK_TYPE_SINGLE, 4, 4, 2.718f, float, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xca\xc0\x2d\xf3\xb6", 5, MSGPACK_TYPE_ANY, MSGPACK_TYPE_SINGLE, 4, 4, -2.718f, float, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xcb\x00\x00\x00\x00\x00\x00\x00\x00", 9, MSGPACK_TYPE_ANY, MSGPACK_TYPE_DOUBLE, 8, 8, 0.0, float, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xcb\x40\x09\x21\xfb\x53\xc8\xd4\xf1", 9, MSGPACK_TYPE_ANY, MSGPACK_TYPE_DOUBLE, 8, 8, 3.14159265, double, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xcb\xc0\x09\x21\xfb\x53\xc8\xd4\xf1", 9, MSGPACK_TYPE_ANY, MSGPACK_TYPE_DOUBLE, 8, 8, -3.14159265, double, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));

    TEST_MSGPACK_READ_CHECK("\xc3", 1, MSGPACK_TYPE_BOOL, MSGPACK_TYPE_BOOL, 1, 1, true, bool, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xc2", 1, MSGPACK_TYPE_BOOL, MSGPACK_TYPE_BOOL, 1, 1, false, bool, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xe0", 1, MSGPACK_TYPE_S8, MSGPACK_TYPE_S8, 1, 1, -32, int8_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\x7f", 1, MSGPACK_TYPE_ANY, MSGPACK_TYPE_U8, 1, 1, 127, uint8_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\x00", 1, MSGPACK_TYPE_U8, MSGPACK_TYPE_U8, 1, 1, 0, uint8_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xcc\x80", 2, MSGPACK_TYPE_U8, MSGPACK_TYPE_U8, 1, 1, 0x80, uint8_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xcc\x00", 2, MSGPACK_TYPE_U8, MSGPACK_TYPE_U8, 1, 1, 0, uint8_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xcd\xa5\x5a", 3, MSGPACK_TYPE_U16, MSGPACK_TYPE_U16, 2, 2, 0xa55a, uint16_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xcd\x00\xa5", 3, MSGPACK_TYPE_U16, MSGPACK_TYPE_U16, 2, 2, 0xa5, uint16_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xce\xa5\x5a\x55\xaa", 5, MSGPACK_TYPE_U32, MSGPACK_TYPE_U32, 4, 4, 0xa55a55aa, uint32_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xce\x00\x00\x00\xa5", 5, MSGPACK_TYPE_U32, MSGPACK_TYPE_U32, 4, 4, 0xa5, uint32_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xcf\xa5\x5a\x55\xaa\xbb\xcc\xdd\xee", 9, MSGPACK_TYPE_U64, MSGPACK_TYPE_U64, 8, 8, 0xa55a55aabbccddee, uint64_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xcf\x00\x00\x00\x00\x00\x00\x00\xa5", 9, MSGPACK_TYPE_U64, MSGPACK_TYPE_U64, 8, 8, 0xa5, uint64_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xd0\xa6", 2, MSGPACK_TYPE_S8, MSGPACK_TYPE_S8, 1, 1, -0x5a, int8_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xd0\x01", 2, MSGPACK_TYPE_S8, MSGPACK_TYPE_S8, 1, 1, 0x01, int8_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xd1\xaa\xa6", 3, MSGPACK_TYPE_S16, MSGPACK_TYPE_S16, 2, 2, -0x555a, int16_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xd1\xff\x5b", 3, MSGPACK_TYPE_S16, MSGPACK_TYPE_S16, 2, 2, -0xa5, int16_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xd2\xaa\xa5\xaa\x56", 5, MSGPACK_TYPE_S32, MSGPACK_TYPE_S32, 4, 4, -0x555a55aa, int32_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xd2\xff\xff\xff\x5b", 5, MSGPACK_TYPE_S32, MSGPACK_TYPE_S32, 4, 4, -0xa5, int32_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xd3\xaa\xa5\xaa\x55\x44\x33\x22\x12", 9, MSGPACK_TYPE_S64, MSGPACK_TYPE_S64, 8, 8, -0x555a55aabbccddee, int64_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xd3\xff\xff\xff\xff\xff\xff\xff\x5b", 9, MSGPACK_TYPE_S64, MSGPACK_TYPE_S64, 8, 8, -0xa5, int64_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xca\x40\x49\x0f\xdb", 5, MSGPACK_TYPE_SINGLE, MSGPACK_TYPE_SINGLE, 4, 4, 3.1415927f, float, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xcb\x40\x09\x21\xfb\x5a\x7e\xd1\x97", 9, MSGPACK_TYPE_DOUBLE, MSGPACK_TYPE_DOUBLE, 8, 8, 3.1415927, double, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xca\x00\x00\x00\x00", 5, MSGPACK_TYPE_SINGLE, MSGPACK_TYPE_SINGLE, 4, 4, 0.0f, float, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xca\x40\x2d\xf3\xb6", 5, MSGPACK_TYPE_SINGLE, MSGPACK_TYPE_SINGLE, 4, 4, 2.718f, float, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xca\xc0\x2d\xf3\xb6", 5, MSGPACK_TYPE_SINGLE, MSGPACK_TYPE_SINGLE, 4, 4, -2.718f, float, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xcb\x00\x00\x00\x00\x00\x00\x00\x00", 9, MSGPACK_TYPE_DOUBLE, MSGPACK_TYPE_DOUBLE, 8, 8, 0.0, float, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xcb\x40\x09\x21\xfb\x53\xc8\xd4\xf1", 9, MSGPACK_TYPE_DOUBLE, MSGPACK_TYPE_DOUBLE, 8, 8, 3.14159265, double, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xcb\xc0\x09\x21\xfb\x53\xc8\xd4\xf1", 9, MSGPACK_TYPE_DOUBLE, MSGPACK_TYPE_DOUBLE, 8, 8, -3.14159265, double, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));

    TEST_MSGPACK_READ_CHECK("\xe0", 1, MSGPACK_TYPE_INTEGER, MSGPACK_TYPE_S8, 1, 1, -32, int8_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\x7f", 1, MSGPACK_TYPE_INTEGER, MSGPACK_TYPE_U8, 1, 1, 127, uint8_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\x00", 1, MSGPACK_TYPE_INTEGER, MSGPACK_TYPE_U8, 1, 1, 0, uint8_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xcc\x80", 2, MSGPACK_TYPE_INTEGER, MSGPACK_TYPE_U8, 1, 1, 0x80, uint8_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xcc\x00", 2, MSGPACK_TYPE_INTEGER, MSGPACK_TYPE_U8, 1, 1, 0, uint8_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xcd\xa5\x5a", 3, MSGPACK_TYPE_INTEGER, MSGPACK_TYPE_U16, 2, 2, 0xa55a, uint16_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xcd\x00\xa5", 3, MSGPACK_TYPE_INTEGER, MSGPACK_TYPE_U16, 2, 2, 0xa5, uint16_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xce\xa5\x5a\x55\xaa", 5, MSGPACK_TYPE_INTEGER, MSGPACK_TYPE_U32, 4, 4, 0xa55a55aa, uint32_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xce\x00\x00\x00\xa5", 5, MSGPACK_TYPE_INTEGER, MSGPACK_TYPE_U32, 4, 4, 0xa5, uint32_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xcf\xa5\x5a\x55\xaa\xbb\xcc\xdd\xee", 9, MSGPACK_TYPE_INTEGER, MSGPACK_TYPE_U64, 8, 8, 0xa55a55aabbccddee, uint64_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xcf\x00\x00\x00\x00\x00\x00\x00\xa5", 9, MSGPACK_TYPE_INTEGER, MSGPACK_TYPE_U64, 8, 8, 0xa5, uint64_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xd0\xa6", 2, MSGPACK_TYPE_INTEGER, MSGPACK_TYPE_S8, 1, 1, -0x5a, int8_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xd0\x01", 2, MSGPACK_TYPE_INTEGER, MSGPACK_TYPE_S8, 1, 1, 0x01, int8_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xd1\xaa\xa6", 3, MSGPACK_TYPE_INTEGER, MSGPACK_TYPE_S16, 2, 2, -0x555a, int16_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xd1\xff\x5b", 3, MSGPACK_TYPE_INTEGER, MSGPACK_TYPE_S16, 2, 2, -0xa5, int16_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xd2\xaa\xa5\xaa\x56", 5, MSGPACK_TYPE_INTEGER, MSGPACK_TYPE_S32, 4, 4, -0x555a55aa, int32_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xd2\xff\xff\xff\x5b", 5, MSGPACK_TYPE_INTEGER, MSGPACK_TYPE_S32, 4, 4, -0xa5, int32_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xd3\xaa\xa5\xaa\x55\x44\x33\x22\x12", 9, MSGPACK_TYPE_INTEGER, MSGPACK_TYPE_S64, 8, 8, -0x555a55aabbccddee, int64_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xd3\xff\xff\xff\xff\xff\xff\xff\x5b", 9, MSGPACK_TYPE_INTEGER, MSGPACK_TYPE_S64, 8, 8, -0xa5, int64_t, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xca\x40\x49\x0f\xdb", 5, MSGPACK_TYPE_FLOAT, MSGPACK_TYPE_SINGLE, 4, 4, 3.1415927f, float, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xcb\x40\x09\x21\xfb\x5a\x7e\xd1\x97", 9, MSGPACK_TYPE_FLOAT, MSGPACK_TYPE_DOUBLE, 8, 8, 3.1415927, double, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xca\x00\x00\x00\x00", 5, MSGPACK_TYPE_FLOAT, MSGPACK_TYPE_SINGLE, 4, 4, 0.0f, float, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xca\x40\x2d\xf3\xb6", 5, MSGPACK_TYPE_FLOAT, MSGPACK_TYPE_SINGLE, 4, 4, 2.718f, float, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xca\xc0\x2d\xf3\xb6", 5, MSGPACK_TYPE_FLOAT, MSGPACK_TYPE_SINGLE, 4, 4, -2.718f, float, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xcb\x00\x00\x00\x00\x00\x00\x00\x00", 9, MSGPACK_TYPE_DOUBLE, MSGPACK_TYPE_DOUBLE, 8, 8, 0.0, float, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xcb\x40\x09\x21\xfb\x53\xc8\xd4\xf1", 9, MSGPACK_TYPE_DOUBLE, MSGPACK_TYPE_DOUBLE, 8, 8, 3.14159265, double, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK("\xcb\xc0\x09\x21\xfb\x53\xc8\xd4\xf1", 9, MSGPACK_TYPE_DOUBLE, MSGPACK_TYPE_DOUBLE, 8, 8, -3.14159265, double, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
}

TEST(msgpack, read_string) {
    TEST_MSGPACK_RSTR_CHECK("\xa0", 1, 1, MSGPACK_TYPE_ANY, MSGPACK_TYPE_STR, 0, "", 0, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_RSTR_CHECK("\xa1t", 2, 2, MSGPACK_TYPE_ANY, MSGPACK_TYPE_STR, 1, "t", 1, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_RSTR_CHECK("\xbftest", 6, 32, MSGPACK_TYPE_ANY, MSGPACK_TYPE_STR, 31, "test", 5, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_RSTR_CHECK("\xd9\xa5test", 7, 0xa7, MSGPACK_TYPE_ANY, MSGPACK_TYPE_STR, 0xa5, "test", 5, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_RSTR_CHECK("\xd9\xfftest", 7, 0x101, MSGPACK_TYPE_ANY, MSGPACK_TYPE_STR, 0xff, "test", 5, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_RSTR_CHECK("\xda\x00\xa5test", 8, 0xa8, MSGPACK_TYPE_ANY, MSGPACK_TYPE_STR, 0xa5, "test", 5, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_RSTR_CHECK("\xda\x02\x00test", 8, 0x203, MSGPACK_TYPE_ANY, MSGPACK_TYPE_STR, 0x200, "test", 5, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_RSTR_CHECK("\xdb\x00\x00\x00\xa5test", 10, 0xaa, MSGPACK_TYPE_ANY, MSGPACK_TYPE_STR, 0xa5, "test", 5, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_RSTR_CHECK("\xdb\x00\x00\x02\xa5test", 10, 0x2aa, MSGPACK_TYPE_ANY, MSGPACK_TYPE_STR, 0x2a5, "test", 5, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));

    TEST_MSGPACK_RSTR_CHECK("\xc4\x00", 2, 2, MSGPACK_TYPE_ANY, MSGPACK_TYPE_BIN, 0, "", 0, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_RSTR_CHECK("\xc4\x01t", 3, 3, MSGPACK_TYPE_ANY, MSGPACK_TYPE_BIN, 1, "t", 1, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_RSTR_CHECK("\xc4\x1ftest", 7, 33, MSGPACK_TYPE_ANY, MSGPACK_TYPE_BIN, 31, "test", 5, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_RSTR_CHECK("\xc4\xa5test", 7, 0xa7, MSGPACK_TYPE_ANY, MSGPACK_TYPE_BIN, 0xa5, "test", 5, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_RSTR_CHECK("\xc4\xfftest", 7, 0x101, MSGPACK_TYPE_ANY, MSGPACK_TYPE_BIN, 0xff, "test", 5, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_RSTR_CHECK("\xc5\x00\xa5test", 8, 0xa8, MSGPACK_TYPE_ANY, MSGPACK_TYPE_BIN, 0xa5, "test", 5, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_RSTR_CHECK("\xc5\x02\x00test", 8, 0x203, MSGPACK_TYPE_ANY, MSGPACK_TYPE_BIN, 0x200, "test", 5, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_RSTR_CHECK("\xc6\x00\x00\x00\xa5test", 10, 0xaa, MSGPACK_TYPE_ANY, MSGPACK_TYPE_BIN, 0xa5, "test", 5, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_RSTR_CHECK("\xc6\x00\x00\x02\xa5test", 10, 0x2aa, MSGPACK_TYPE_ANY, MSGPACK_TYPE_BIN, 0x2a5, "test", 5, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));

    TEST_MSGPACK_RSTR_CHECK("\xa0", 1, 1, MSGPACK_TYPE_STR, MSGPACK_TYPE_STR, 0, "", 0, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_RSTR_CHECK("\xa1t", 2, 2, MSGPACK_TYPE_STR, MSGPACK_TYPE_STR, 1, "t", 1, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_RSTR_CHECK("\xbftest", 6, 32, MSGPACK_TYPE_STR, MSGPACK_TYPE_STR, 31, "test", 5, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_RSTR_CHECK("\xd9\xa5test", 7, 0xa7, MSGPACK_TYPE_STR, MSGPACK_TYPE_STR, 0xa5, "test", 5, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_RSTR_CHECK("\xd9\xfftest", 7, 0x101, MSGPACK_TYPE_STR, MSGPACK_TYPE_STR, 0xff, "test", 5, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_RSTR_CHECK("\xda\x00\xa5test", 8, 0xa8, MSGPACK_TYPE_STR, MSGPACK_TYPE_STR, 0xa5, "test", 5, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_RSTR_CHECK("\xda\x02\x00test", 8, 0x203, MSGPACK_TYPE_STR, MSGPACK_TYPE_STR, 0x200, "test", 5, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_RSTR_CHECK("\xdb\x00\x00\x00\xa5test", 10, 0xaa, MSGPACK_TYPE_STR, MSGPACK_TYPE_STR, 0xa5, "test", 5, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_RSTR_CHECK("\xdb\x00\x00\x02\xa5test", 10, 0x2aa, MSGPACK_TYPE_STR, MSGPACK_TYPE_STR, 0x2a5, "test", 5, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));

    TEST_MSGPACK_RSTR_CHECK("\xc4\x00", 2, 2, MSGPACK_TYPE_BIN, MSGPACK_TYPE_BIN, 0, "", 0, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_RSTR_CHECK("\xc4\x01t", 3, 3, MSGPACK_TYPE_BIN, MSGPACK_TYPE_BIN, 1, "t", 1, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_RSTR_CHECK("\xc4\x1ftest", 7, 33, MSGPACK_TYPE_BIN, MSGPACK_TYPE_BIN, 31, "test", 5, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_RSTR_CHECK("\xc4\xa5test", 7, 0xa7, MSGPACK_TYPE_BIN, MSGPACK_TYPE_BIN, 0xa5, "test", 5, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_RSTR_CHECK("\xc4\xfftest", 7, 0x101, MSGPACK_TYPE_BIN, MSGPACK_TYPE_BIN, 0xff, "test", 5, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_RSTR_CHECK("\xc5\x00\xa5test", 8, 0xa8, MSGPACK_TYPE_BIN, MSGPACK_TYPE_BIN, 0xa5, "test", 5, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_RSTR_CHECK("\xc5\x02\x00test", 8, 0x203, MSGPACK_TYPE_BIN, MSGPACK_TYPE_BIN, 0x200, "test", 5, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_RSTR_CHECK("\xc6\x00\x00\x00\xa5test", 10, 0xaa, MSGPACK_TYPE_BIN, MSGPACK_TYPE_BIN, 0xa5, "test", 5, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
    TEST_MSGPACK_RSTR_CHECK("\xc6\x00\x00\x02\xa5test", 10, 0x2aa, MSGPACK_TYPE_BIN, MSGPACK_TYPE_BIN, 0x2a5, "test", 5, msgpack_unpack(&test_unpacker, &test_data, &test_data_len, &test_type));
}

TEST(msgpack, read_len) {
    TEST_MSGPACK_READ_CHECK_LEN("\xc3", 1, MSGPACK_TYPE_ANY, MSGPACK_TYPE_BOOL, 1, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xc2", 1, MSGPACK_TYPE_ANY, MSGPACK_TYPE_BOOL, 1, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xe0", 1, MSGPACK_TYPE_ANY, MSGPACK_TYPE_S8, 1, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\x7f", 1, MSGPACK_TYPE_ANY, MSGPACK_TYPE_U8, 1, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\x00", 1, MSGPACK_TYPE_ANY, MSGPACK_TYPE_U8, 1, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xcc\x80", 2, MSGPACK_TYPE_ANY, MSGPACK_TYPE_U8, 1, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xcc\x00", 2, MSGPACK_TYPE_ANY, MSGPACK_TYPE_U8, 1, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xcd\xa5\x5a", 3, MSGPACK_TYPE_ANY, MSGPACK_TYPE_U16, 2, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xcd\x00\xa5", 3, MSGPACK_TYPE_ANY, MSGPACK_TYPE_U16, 2, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xce\xa5\x5a\x55\xaa", 5, MSGPACK_TYPE_ANY, MSGPACK_TYPE_U32, 4, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xce\x00\x00\x00\xa5", 5, MSGPACK_TYPE_ANY, MSGPACK_TYPE_U32, 4, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xcf\xa5\x5a\x55\xaa\xbb\xcc\xdd\xee", 9, MSGPACK_TYPE_ANY, MSGPACK_TYPE_U64, 8, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xcf\x00\x00\x00\x00\x00\x00\x00\xa5", 9, MSGPACK_TYPE_ANY, MSGPACK_TYPE_U64, 8, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xd0\xa6", 2, MSGPACK_TYPE_ANY, MSGPACK_TYPE_S8, 1, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xd0\x01", 2, MSGPACK_TYPE_ANY, MSGPACK_TYPE_S8, 1, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xd1\xaa\xa6", 3, MSGPACK_TYPE_ANY, MSGPACK_TYPE_S16, 2, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xd1\xff\x5b", 3, MSGPACK_TYPE_ANY, MSGPACK_TYPE_S16, 2, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xd2\xaa\xa5\xaa\x56", 5, MSGPACK_TYPE_ANY, MSGPACK_TYPE_S32, 4, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xd2\xff\xff\xff\x5b", 5, MSGPACK_TYPE_ANY, MSGPACK_TYPE_S32, 4, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xd3\xaa\xa5\xaa\x55\x44\x33\x22\x12", 9, MSGPACK_TYPE_ANY, MSGPACK_TYPE_S64, 8, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xd3\xff\xff\xff\xff\xff\xff\xff\x5b", 9, MSGPACK_TYPE_ANY, MSGPACK_TYPE_S64, 8, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xca\x40\x49\x0f\xdb", 5, MSGPACK_TYPE_ANY, MSGPACK_TYPE_SINGLE, 4, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xcb\x40\x09\x21\xfb\x5a\x7e\xd1\x97", 9, MSGPACK_TYPE_ANY, MSGPACK_TYPE_DOUBLE, 8, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xca\x00\x00\x00\x00", 5, MSGPACK_TYPE_ANY, MSGPACK_TYPE_SINGLE, 4, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xca\x40\x2d\xf3\xb6", 5, MSGPACK_TYPE_ANY, MSGPACK_TYPE_SINGLE, 4, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xca\xc0\x2d\xf3\xb6", 5, MSGPACK_TYPE_ANY, MSGPACK_TYPE_SINGLE, 4, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xcb\x00\x00\x00\x00\x00\x00\x00\x00", 9, MSGPACK_TYPE_ANY, MSGPACK_TYPE_DOUBLE, 8, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xcb\x40\x09\x21\xfb\x53\xc8\xd4\xf1", 9, MSGPACK_TYPE_ANY, MSGPACK_TYPE_DOUBLE, 8, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xcb\xc0\x09\x21\xfb\x53\xc8\xd4\xf1", 9, MSGPACK_TYPE_ANY, MSGPACK_TYPE_DOUBLE, 8, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));

    TEST_MSGPACK_READ_CHECK_LEN("\xa0", 1, MSGPACK_TYPE_ANY, MSGPACK_TYPE_STR, 0, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xa1t", 2, MSGPACK_TYPE_ANY, MSGPACK_TYPE_STR, 1, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xbftest", 6, MSGPACK_TYPE_ANY, MSGPACK_TYPE_STR, 31, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xd9\xa5test", 7, MSGPACK_TYPE_ANY, MSGPACK_TYPE_STR, 0xa5, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xd9\xfftest", 7, MSGPACK_TYPE_ANY, MSGPACK_TYPE_STR, 0xff, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xda\x00\xa5test", 8, MSGPACK_TYPE_ANY, MSGPACK_TYPE_STR, 0xa5, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xda\x02\x00test", 8, MSGPACK_TYPE_ANY, MSGPACK_TYPE_STR, 0x200, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xdb\x00\x00\x00\xa5test", 10, MSGPACK_TYPE_ANY, MSGPACK_TYPE_STR, 0xa5, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xdb\x00\x00\x02\xa5test", 10, MSGPACK_TYPE_ANY, MSGPACK_TYPE_STR, 0x2a5, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));

    TEST_MSGPACK_READ_CHECK_LEN("\xc4\x00", 2, MSGPACK_TYPE_ANY, MSGPACK_TYPE_BIN, 0, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xc4\x01t", 3, MSGPACK_TYPE_ANY, MSGPACK_TYPE_BIN, 1, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xc4\x1ftest", 7, MSGPACK_TYPE_ANY, MSGPACK_TYPE_BIN, 31, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xc4\xa5test", 7, MSGPACK_TYPE_ANY, MSGPACK_TYPE_BIN, 0xa5, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xc4\xfftest", 7, MSGPACK_TYPE_ANY, MSGPACK_TYPE_BIN, 0xff, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xc5\x00\xa5test", 8, MSGPACK_TYPE_ANY, MSGPACK_TYPE_BIN, 0xa5, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xc5\x02\x00test", 8, MSGPACK_TYPE_ANY, MSGPACK_TYPE_BIN, 0x200, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xc6\x00\x00\x00\xa5test", 10, MSGPACK_TYPE_ANY, MSGPACK_TYPE_BIN, 0xa5, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
    TEST_MSGPACK_READ_CHECK_LEN("\xc6\x00\x00\x02\xa5test", 10, MSGPACK_TYPE_ANY, MSGPACK_TYPE_BIN, 0x2a5, msgpack_unpack(&test_unpacker, NULL, &test_data_len, &test_type));
}

TEST(msgpack, error_call) {
    uint8_t buf[1024] = {0};
    msgpack_buffer_t mbuf;
    char data[] = "test";

    TEST_MSGPACK_WRITE_CHECK_ERROR(MSGPACK_EINVL, &mbuf, buf, 1024, msgpack_byte(NULL, NIL_TAG));
    TEST_MSGPACK_WRITE_CHECK_ERROR(MSGPACK_EINIT, &mbuf, NULL, 0, msgpack_byte(&mbuf, NIL_TAG));
    TEST_MSGPACK_WRITE_CHECK_ERROR(MSGPACK_ENOBUF, &mbuf, buf, 0, msgpack_byte(&mbuf, NIL_TAG));
    TEST_MSGPACK_WRITE_CHECK_ERROR(MSGPACK_EINVL, &mbuf, buf, 1024, msgpack_data(NULL, U8_TAG, data, 1));
    TEST_MSGPACK_WRITE_CHECK_ERROR(MSGPACK_EINVL, &mbuf, buf, 1024, msgpack_data(&mbuf, U8_TAG, NULL, 1));
    TEST_MSGPACK_WRITE_CHECK_ERROR(MSGPACK_EINVL, &mbuf, buf, 1024, msgpack_data(&mbuf, U8_TAG, data, 0));
    TEST_MSGPACK_WRITE_CHECK_ERROR(MSGPACK_EINIT, &mbuf, NULL, 0, msgpack_data(&mbuf, U8_TAG, data, 1));
    TEST_MSGPACK_WRITE_CHECK_ERROR(MSGPACK_ENOBUF, &mbuf, buf, 0, msgpack_data(&mbuf, U8_TAG, data, 1));
    TEST_MSGPACK_WRITE_CHECK_ERROR(MSGPACK_ENOBUF, &mbuf, buf, 1, msgpack_data(&mbuf, U8_TAG, data, 1));
    TEST_MSGPACK_WRITE_CHECK_ERROR(MSGPACK_EINVL, &mbuf, buf, 1024, msgpack_len_data(NULL, STR16_TAG, data, 1, 2));
    TEST_MSGPACK_WRITE_CHECK_ERROR(MSGPACK_EINVL, &mbuf, buf, 1024, msgpack_len_data(&mbuf, STR16_TAG, NULL, 1, 2));
    TEST_MSGPACK_WRITE_CHECK_ERROR(MSGPACK_EINIT, &mbuf, NULL, 0, msgpack_len_data(&mbuf, STR16_TAG, data, 1, 2));
    TEST_MSGPACK_WRITE_CHECK_ERROR(MSGPACK_ENOBUF, &mbuf, buf, 0, msgpack_len_data(&mbuf, STR16_TAG, data, 1, 2));
    TEST_MSGPACK_WRITE_CHECK_ERROR(MSGPACK_ENOBUF, &mbuf, buf, 2, msgpack_len_data(&mbuf, STR16_TAG, data, 1, 2));

    msgpack_unpacker_t unpacker;
    uint8_t type;
    uint32_t data_len;
    TEST_MSGPACK_READ_CHECK_ERROR(MSGPACK_EINVL, &unpacker, buf, 1024, 0, msgpack_unpack(NULL, data, &data_len, &type), data_len=5;type=MSGPACK_TYPE_ANY);
    TEST_MSGPACK_READ_CHECK_ERROR(MSGPACK_EINVL, &unpacker, buf, 1024, 0, msgpack_unpack(&unpacker, NULL, NULL, &type), data_len=5;type=MSGPACK_TYPE_ANY);
    TEST_MSGPACK_READ_CHECK_ERROR(MSGPACK_EINVL, &unpacker, buf, 1024, 0, msgpack_unpack(&unpacker, &data, NULL, &type), data_len=5;type=MSGPACK_TYPE_ANY);
    TEST_MSGPACK_READ_CHECK_ERROR(MSGPACK_EINVL, &unpacker, buf, 1024, 0, msgpack_unpack(&unpacker, data, &data_len, NULL), data_len=5;type=MSGPACK_TYPE_ANY);
    TEST_MSGPACK_READ_CHECK_ERROR(MSGPACK_EINIT, &unpacker, NULL, 0, 0, msgpack_unpack(&unpacker, data, &data_len, &type), data_len=5;type=MSGPACK_TYPE_ANY);
    TEST_MSGPACK_READ_CHECK_ERROR(MSGPACK_EENDBUF, &unpacker, buf, 0, 0, msgpack_unpack(&unpacker, data, &data_len, &type), data_len=5;type=MSGPACK_TYPE_ANY);
    TEST_MSGPACK_READ_CHECK_ERROR(MSGPACK_EENDBUF, &unpacker, buf, 10, 10, msgpack_unpack(&unpacker, data, &data_len, &type), data_len=5;type=MSGPACK_TYPE_ANY);
    TEST_MSGPACK_READ_CHECK_ERROR(MSGPACK_EINVL, &unpacker, buf, 1024, 0, msgpack_unpack(&unpacker, data, &data_len, &type), data_len=0;type=MSGPACK_TYPE_ANY);
    TEST_MSGPACK_READ_CHECK_ERROR(MSGPACK_EENDBUF, &unpacker, buf, 1, 0, msgpack_unpack(&unpacker, data, &data_len, &type), memcpy(buf, "\xcc", 1);data_len=5;type=MSGPACK_TYPE_ANY);
    TEST_MSGPACK_READ_CHECK_ERROR(MSGPACK_EENDBUF, &unpacker, buf, 2, 0, msgpack_unpack(&unpacker, data, &data_len, &type), memcpy(buf, "\xd9\x02", 2);data_len=5;type=MSGPACK_TYPE_ANY);
    TEST_MSGPACK_READ_CHECK_ERROR(MSGPACK_EENDBUF, &unpacker, buf, 1, 0, msgpack_unpack(&unpacker, data, &data_len, &type), memcpy(buf, "\xd9\x02", 2);data_len=5;type=MSGPACK_TYPE_ANY);
    TEST_MSGPACK_READ_CHECK_ERROR(MSGPACK_ENOBUF, &unpacker, buf, 4, 0, msgpack_unpack(&unpacker, data, &data_len, &type), memcpy(buf, "\xd9\x02", 2);data_len=1;type=MSGPACK_TYPE_ANY);
    TEST_MSGPACK_READ_CHECK_ERROR(MSGPACK_EUNEXPECTED, &unpacker, buf, 1024, 0, msgpack_unpack(&unpacker, data, &data_len, &type), memcpy(buf, "\xc0", 1);data_len=5;type=MSGPACK_TYPE_BOOL);
    TEST_MSGPACK_READ_CHECK_ERROR(MSGPACK_EUNEXPECTED, &unpacker, buf, 1024, 0, msgpack_unpack(&unpacker, data, &data_len, &type), memcpy(buf, "\xc2", 1);data_len=5;type=MSGPACK_TYPE_NIL);
    TEST_MSGPACK_READ_CHECK_ERROR(MSGPACK_EUNEXPECTED, &unpacker, buf, 1024, 0, msgpack_unpack(&unpacker, data, &data_len, &type), memcpy(buf, "\xd9\x02", 2);data_len=5;type=MSGPACK_TYPE_INTEGER);
    TEST_MSGPACK_READ_CHECK_ERROR(MSGPACK_EUNEXPECTED, &unpacker, buf, 1024, 0, msgpack_unpack(&unpacker, data, &data_len, &type), memcpy(buf, "\xd9\x02", 2);data_len=5;type=MSGPACK_TYPE_FLOAT);
    TEST_MSGPACK_READ_CHECK_ERROR(MSGPACK_EUNEXPECTED, &unpacker, buf, 1024, 0, msgpack_unpack(&unpacker, data, &data_len, &type), memcpy(buf, "\xcc\x02", 2);data_len=5;type=MSGPACK_TYPE_STR);
    TEST_MSGPACK_READ_CHECK_ERROR(MSGPACK_EUNKNOWN, &unpacker, buf, 1024, 0, msgpack_unpack(&unpacker, data, &data_len, &type), memcpy(buf, "\xde", 1);data_len=5;type=MSGPACK_TYPE_ANY);
}
