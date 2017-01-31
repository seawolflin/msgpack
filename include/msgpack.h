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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef MSGPACK_H
#define MSGPACK_H

enum {
  MSGPACK_EOK = 0,
  MSGPACK_EUNKNOWN,
  MSGPACK_EINVL,
  MSGPACK_ENOBUF,
  MSGPACK_EINIT,
  MSGPACK_EENDBUF,
  MSGPACK_EUNEXPECTED,
  MSGPACK_EMAX
};


#define POS_FIXNUM_TAG  0x00 // 0x00 ~ 0x7f (0 ~ 127)
#define FIXMAP_TAG           0x80 // 0x80 ~ 0x8f
#define FIXARRAY_TAG         0x90 // 0x90 ~ 0x9f
#define FIXSTR_TAG           0xA0 // 0xa0 ~ 0xbf
#define NIL_TAG              0xC0
#define FALSE_TAG            0xC2
#define TRUE_TAG             0xC3
#define BIN8_TAG             0xC4
#define BIN16_TAG            0xC5
#define BIN32_TAG            0xC6
#define EXT8_TAG             0xC7
#define EXT16_TAG            0xC8
#define EXT32_TAG            0xC9
#define FLOAT_TAG            0xCA
#define DOUBLE_TAG           0xCB
#define U8_TAG               0xCC
#define U16_TAG              0xCD
#define U32_TAG              0xCE
#define U64_TAG              0xCF
#define S8_TAG               0xD0
#define S16_TAG              0xD1
#define S32_TAG              0xD2
#define S64_TAG              0xD3
#define FIXEXT1_TAG          0xD4
#define FIXEXT2_TAG          0xD5
#define FIXEXT4_TAG          0xD6
#define FIXEXT8_TAG          0xD7
#define FIXEXT16_TAG         0xD8
#define STR8_TAG             0xD9
#define STR16_TAG            0xDA
#define STR32_TAG            0xDB
#define ARRAY16_TAG          0xDC
#define ARRAY32_TAG          0xDD
#define MAP16_TAG            0xDE
#define MAP32_TAG            0xDF
#define NEG_FIXNUM_TAG  0xE0 // 0xE0 ~ 0xFF (-32 ~ -1)

enum {
  MSGPACK_TYPE_ANY     = 0x00,
  MSGPACK_TYPE_NIL     = 0x10,
  MSGPACK_TYPE_BOOL    = 0x20,
  MSGPACK_TYPE_INTEGER = 0x30, /* from -(2^63) upto (2^63)-1, can store in int64_t */
  MSGPACK_TYPE_S8      = 0x31,
  MSGPACK_TYPE_U8      = 0x32,
  MSGPACK_TYPE_S16     = 0x33,
  MSGPACK_TYPE_U16     = 0x34,
  MSGPACK_TYPE_S32     = 0x35,
  MSGPACK_TYPE_U32     = 0x36,
  MSGPACK_TYPE_S64     = 0x37,
  MSGPACK_TYPE_U64     = 0x38, /* from 0 to (2^64)-1 */
  MSGPACK_TYPE_FLOAT   = 0x40, /* both single and double float */
  MSGPACK_TYPE_SINGLE  = 0x41,
  MSGPACK_TYPE_DOUBLE  = 0x42,
  MSGPACK_TYPE_STR     = 0x50,
  MSGPACK_TYPE_BIN     = 0x60,
  // MSGPACK_TYPE_ARRAY,   /* 15 */
  // MSGPACK_TYPE_MAP,     /* 16 */
  // MSGPACK_TYPE_EXT,     /* 17 */
};

#ifdef  __cplusplus
extern "C"
{
#endif

int msgpack_errno(void);
#if !defined(MSGPACK_SIZE_OPT)
const char *msgpack_errmsg(void);
#else
#define msgpack_errmsg ""
#endif

#ifdef __cplusplus
}
#endif

/**
 * @name Writer
 * @{
 */

typedef struct msgpack_buffer msgpack_buffer_t;

#define init_msgpack_buffer(mbuf, b, l) \
  do { \
    (mbuf)->buf = (b); \
    (mbuf)->len = 0; \
    (mbuf)->alloc = (l); \
  } while(0)


struct msgpack_buffer{
  uint8_t *buf;
  size_t len;
  size_t alloc;
};

#ifdef  __cplusplus
extern "C"
{
#endif

bool msgpack_byte(msgpack_buffer_t *mbuf, uint8_t data);
bool msgpack_data(msgpack_buffer_t *mbuf, uint8_t type, const void *data, size_t len);
bool msgpack_len_data(msgpack_buffer_t *mbuf, uint8_t type, const void *data,
        uint32_t len, size_t len_size);
bool msgpack_data_uinteger(struct msgpack_buffer *mbuf, uint8_t type,
        const uint64_t data, size_t len);
bool msgpack_data_sinteger(struct msgpack_buffer *mbuf, uint8_t type,
        const int64_t data, size_t len);
bool msgpack_data_float(struct msgpack_buffer *mbuf, uint8_t type,
        const float data, size_t len);

bool msgpack_data_double(struct msgpack_buffer *mbuf, uint8_t type,
        const double data, size_t len);

#ifdef __cplusplus
}
#endif

#define msgpack_write_nil(mbuf) \
  msgpack_byte(mbuf, NIL_TAG)
#define msgpack_write_bool(mbuf, val) \
  msgpack_byte(mbuf, val == true ? TRUE_TAG : FALSE_TAG)
#define msgpack_write_true(mbuf) \
  msgpack_byte(mbuf, TRUE_TAG)
#define msgpack_write_false(mbuf) \
  msgpack_byte(mbuf, FALSE_TAG)

/**
 * @name Integers
 * @{
 */

// write small integer[-32, 127]
#define msgpack_write_smallint(mbuf, val) \
  msgpack_byte(mbuf, val)
#define msgpack_write_u8(mbuf, val) \
  msgpack_data_uinteger(mbuf, U8_TAG, val, 1)
#define msgpack_write_u16(mbuf, val) \
  msgpack_data_uinteger(mbuf, U16_TAG, val, 2)
#define msgpack_write_u32(mbuf, val) \
  msgpack_data_uinteger(mbuf, U32_TAG, val, 4)
#define msgpack_write_u64(mbuf, val) \
  msgpack_data_uinteger(mbuf, U64_TAG, val, 8)
#define msgpack_write_s8(mbuf, val) \
  msgpack_data_sinteger(mbuf, S8_TAG, val, 1)
#define msgpack_write_s16(mbuf, val) \
  msgpack_data_sinteger(mbuf, S16_TAG, val, 2)
#define msgpack_write_s32(mbuf, val) \
  msgpack_data_sinteger(mbuf, S32_TAG, val, 4)
#define msgpack_write_s64(mbuf, val) \
  msgpack_data_sinteger(mbuf, S64_TAG, val, 8)
#define msgpack_uint_valsize(val) (\
  ((val) >> 8 == 0) ? 1 : \
  ((val) >> 16 == 0) ? 2 : \
  ((val) >> 32 == 0) ? 4 : 8)
#define msgpack_sint_valsize(val) (\
  ((-val) >> 7 == 0) ? 1 : \
  ((-val) >> 15 == 0) ? 2 : \
  ((-val) >> 31 == 0) ? 4 : 8)
#define msgpack_uint_type(val) (\
  ((val) >> 8 == 0) ? U8_TAG : \
  ((val) >> 16 == 0) ? U16_TAG : \
  ((val) >> 32 == 0) ? U32_TAG : U64_TAG)
#define msgpack_sint_type(val) (\
  ((-val) >> 7 == 0) ? S8_TAG : \
  ((-val) >> 15 == 0) ? S16_TAG : \
  ((-val) >> 31 == 0) ? S32_TAG : S64_TAG)
#define msgpack_write_integer(mbuf, val) (\
  ((val) < 0) ? \
    (((val) >= -32) ? msgpack_byte(mbuf, val) : \
      msgpack_data_sinteger(mbuf, msgpack_sint_type(val), val, \
        msgpack_sint_valsize(val))) \
    : (((val) >> 7 == 0) ? msgpack_byte(mbuf, val) : \
      msgpack_data_uinteger(mbuf, msgpack_uint_type((uint64_t)val), val, \
        msgpack_uint_valsize((uint64_t)val))))
/**
 * @}
 */

/**
 * @name Float
 * @{
 */

#define msgpack_write_float(mbuf, val) \
  msgpack_data_float(mbuf, FLOAT_TAG, val, 4)
#define msgpack_write_double(mbuf, val) \
  msgpack_data_double(mbuf, DOUBLE_TAG, val, 8)

/**
 * @}
 */

/**
 * @name String
 * @{
 */
// len from 0 to 31
#define msgpack_write_smallstr(mbuf, str, len) \
  msgpack_len_data(mbuf, FIXSTR_TAG, str, len, 0)
#define msgpack_write_str8(mbuf, str, len) \
  msgpack_len_data(mbuf, STR8_TAG, str, len, 1)
#define msgpack_write_str16(mbuf, str, len) \
  msgpack_len_data(mbuf, STR16_TAG, str, len, 2)
#define msgpack_write_str32(mbuf, str, len) \
  msgpack_len_data(mbuf, STR32_TAG, str, len, 4)
#define msgpack_str_lensize(len) ((len) >> 5 == 0) ? 0 : \
  ((len) >> 8 == 0) ? 1 : \
  ((len) >> 16 == 0) ? 2 : 4
#define msgpack_str_type(len) ((len) >> 5 == 0) ? FIXSTR_TAG : \
  ((len) >> 8 == 0) ? STR8_TAG : \
  ((len) >> 16 == 0) ? STR16_TAG : STR32_TAG
#define msgpack_write_str(mbuf, str, len) \
  msgpack_len_data(mbuf, msgpack_str_type(len), str, len, msgpack_str_lensize(len))

/**
 * @}
 */

/**
 * @name Binary
 * @{
 */

#define msgpack_write_bin8(mbuf, bin, len) \
  msgpack_len_data(mbuf, BIN8_TAG, bin, len, 1)
#define msgpack_write_bin16(mbuf, bin, len) \
  msgpack_len_data(mbuf, BIN16_TAG, bin, len, 2)
#define msgpack_write_bin32(mbuf, bin, len) \
  msgpack_len_data(mbuf, BIN32_TAG, bin, len, 4)
#define msgpack_bin_lensize(len) ((len) >> 8 == 0) ? 1 : \
  ((len) >> 16 == 0) ? 2 : 4
#define msgpack_bin_type(len) ((len) >> 8 == 0) ? BIN8_TAG : \
  ((len) >> 16 == 0) ? BIN16_TAG : BIN32_TAG
#define msgpack_write_bin(mbuf, bin, len) \
  msgpack_len_data(mbuf, msgpack_bin_type(len), bin, len, msgpack_bin_lensize(len))

/**
 * @}
 */

/**
 * @}
 */

/**
 * @name Reader
 * @{
 */

typedef struct msgpack_unpacker msgpack_unpacker_t;

#define init_msgpack_unpacker(upr, b, l, used) \
  do { \
    (upr)->buf = (b); \
    (upr)->len = (l); \
    (upr)->pos = (used); \
  } while(0)

struct msgpack_unpacker{
  uint8_t *buf;
  size_t pos;
  size_t len;
};

#ifdef  __cplusplus
extern "C"
{
#endif

bool msgpack_unpack(struct msgpack_unpacker *up, void *data, uint32_t *data_len,
        uint8_t *type);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif
