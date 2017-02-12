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

#include <string.h>

#include "msgpack.h"

#define FIX_TAG_MASK 0xC0
#define FIX_TAG      0x80 /* (FIXMAP_TAG || FIXARRAY_TAG || FIXSTR_TAG) */
#define FIXSTR_TAG_MASK 0xE0

static int g_errno = 0;

inline int msgpack_errno(void) {
  return g_errno;
}

static void set_errno(int errno) {
  g_errno = errno;
}

#if !MSGPACK_SIZE_OPT
const char *g_errmsgs[MSGPACK_EMAX] = {
  "No Error",
  "Unknown Error",
  "Invlid Parameters",
  "Buffer is too short",
  "No Initial",
  "Reach the buffer end",
  "Read a unexpect type",
};

const char *msgpack_errmsg(void) {
  return g_errmsgs[g_errno];
}
#endif

#define type_mask(t) (0xF0 & (t))
static const int32_t _i = 1;
#define is_bigendian() ((*(char *)&_i) == 0)

bool msgpack_byte(struct msgpack_buffer *mbuf, uint8_t data) {
  if (!mbuf) {
    set_errno(MSGPACK_EINVL);
    goto error;
  }

  if (!mbuf->buf) {
    set_errno(MSGPACK_EINIT);
    goto error;
  }

  set_errno(MSGPACK_EOK);
  if (mbuf->alloc <= mbuf->len) {
    set_errno(MSGPACK_ENOBUF);
    goto error;
  }
  mbuf->buf[mbuf->len++] = data;
  return true;

error:
  return false;
}

static void msgpack_endiancpy(void *buffer, const void *data, size_t len) {
  int i;
  if (is_bigendian()) {
    memcpy(buffer, data, len);
  }
  for (i = len - 1; i >= 0; --i) {
    ((uint8_t *)buffer)[len - i - 1] = ((uint8_t *)data)[i];
  }
}

bool msgpack_data(struct msgpack_buffer *mbuf, uint8_t type,
        const void *data, size_t len) {
  if (!mbuf || !data || len == 0) {
    set_errno(MSGPACK_EINVL);
    goto error;
  }

  if (!mbuf->buf) {
    set_errno(MSGPACK_EINIT);
    goto error;
  }

  set_errno(MSGPACK_EOK);
  if (mbuf->alloc <= mbuf->len + len) {
    set_errno(MSGPACK_ENOBUF);
    goto error;
  }
  mbuf->buf[mbuf->len++] = type;
  msgpack_endiancpy(mbuf->buf + mbuf->len, data, len);
  mbuf->len += len;

  return true;
error:
  return false;
}

bool msgpack_len_data(struct msgpack_buffer *mbuf, uint8_t type, const void *data,
        uint32_t len, size_t len_size) {
  int i;
  int index;

  if (!mbuf) {
    set_errno(MSGPACK_EINVL);
    goto error;
  }

  if (!mbuf->buf) {
    set_errno(MSGPACK_EINIT);
    goto error;
  }

  set_errno(MSGPACK_EOK);
  if (mbuf->alloc <= mbuf->len + len_size + (data ? len : 0)) {
    set_errno(MSGPACK_ENOBUF);
    goto error;
  }

  if ((type & FIX_TAG_MASK) == FIX_TAG) {
    type |= (uint8_t)len;
  }
  mbuf->buf[mbuf->len++] = type;
  for (i = 0; i < len_size; ++i) {
    index = len_size - i - 1;
    if (is_bigendian()) {
      index = i;
    }
    mbuf->buf[mbuf->len++] = ((uint8_t *)&len)[index];
  }
  if (data) {
    memcpy(mbuf->buf + mbuf->len, data, len);
    mbuf->len += len;
  }
  return true;

error:
  return false;
}

bool msgpack_data_uinteger(struct msgpack_buffer *mbuf, uint8_t type,
        const uint64_t data, size_t len) {
  uint64_t temp = data;
  return msgpack_data(mbuf, type, &temp, len);
}

bool msgpack_data_sinteger(struct msgpack_buffer *mbuf, uint8_t type,
        const int64_t data, size_t len) {
  int64_t temp = data;
  return msgpack_data(mbuf, type, &temp, len);
}

bool msgpack_data_float(struct msgpack_buffer *mbuf, uint8_t type,
        const float data, size_t len) {
  float temp = data;
  return msgpack_data(mbuf, type, &temp, len);
}

bool msgpack_data_double(struct msgpack_buffer *mbuf, uint8_t type,
        const double data, size_t len) {
  double temp = data;
  return msgpack_data(mbuf, type, &temp, len);
}

static inline uint8_t msgpack_read_byte(struct msgpack_unpacker *up) {
  return up->buf[up->pos++];
}

static bool msgpack_read(struct msgpack_unpacker *up, void *data, size_t len) {
  if (up->pos + len > up->len) {
    set_errno(MSGPACK_EENDBUF);
    return false;
  }

  memcpy(data, up->buf + up->pos, len);
  up->pos += len;
  return true;
}

static bool msgpack_read_len(struct msgpack_unpacker *up, uint32_t *len,
        size_t len_size) {
  int i;
  int index;

  if (up->pos + len_size > up->len) {
    set_errno(MSGPACK_EENDBUF);
    return false;
  }

  for (i = 0; i < len_size; ++i) {
    index = len_size - i - 1;
    if (is_bigendian()) {
      index = i;
    }
    ((uint8_t *)len)[index] = up->buf[up->pos++];
  }
  return true;
}

static bool msgpack_read_endian(struct msgpack_unpacker *up,
        void *data, size_t len) {
  if (up->pos + len > up->len) {
    set_errno(MSGPACK_EENDBUF);
    return false;
  }

  msgpack_endiancpy(data, up->buf + up->pos, len);
  up->pos += len;
  return true;
}

bool msgpack_unpack(struct msgpack_unpacker *up, void *data, uint32_t *data_len,
        uint8_t *type) {
  uint8_t head;
  uint32_t len = 0;
  bool has_len;
  size_t len_size;
  bool endian;
  bool len_ok = false;
  uint8_t m_type = MSGPACK_TYPE_ANY;
  size_t read = 0;
  bool ret;

  if (!up || !type) {
    set_errno(MSGPACK_EINVL);
    goto error;
  }

  if (!up->buf) {
    set_errno(MSGPACK_EINIT);
    goto error;
  }

  if (up->pos >= up->len) {
    set_errno(MSGPACK_EENDBUF);
    goto error;
  }

  set_errno(MSGPACK_EOK);
  head = msgpack_read_byte(up);
  read++;

  /* If the byte is NIL, data and data_len can be NULL. */
  if (head == NIL_TAG) {
    m_type = MSGPACK_TYPE_NIL;
    if (*type != MSGPACK_TYPE_ANY && type_mask(*type) != type_mask(m_type)) {
      set_errno(MSGPACK_EUNEXPECTED);
      goto error;
    }
    goto exit;
  }

  /**
   * If the byte is not NIL:
   * 1. data = NULL, data_len != NULL, it will return data len to be read, but not
   * read really.
   * 2. data != NULL, data_len != NULL, data_len is the data buffer len.
   * 3. data_len = NULL or *data_len < 1, is invalid parameters.
   */
  if (!data_len || (data && *data_len < 1)) {
    set_errno(MSGPACK_EINVL);
    goto error;
  }

  if ((head & FIXSTR_TAG_MASK) == FIXSTR_TAG) {
    len = head & 0x1F;
    head = FIXSTR_TAG;
  } else if ((head & FIX_TAG_MASK) == FIX_TAG) {
    len = head & 0x0F;
    head = head & 0xF0;
  } else if ((head & 0x80) == POS_FIXNUM_TAG) { /* positive fixnum */
    m_type = MSGPACK_TYPE_U8;
    goto read_head_data;
  } else if ((head & 0xE0) == NEG_FIXNUM_TAG) { /* negative fixnum */
    m_type = MSGPACK_TYPE_S8;
    goto read_head_data;
  } else if (head == FALSE_TAG || head == TRUE_TAG) {
    m_type = MSGPACK_TYPE_BOOL;
    head = (head == TRUE_TAG) ? (uint8_t)true : (uint8_t)false;
read_head_data:
    if (*type != MSGPACK_TYPE_ANY && type_mask(*type) != type_mask(m_type)) {
      set_errno(MSGPACK_EUNEXPECTED);
      goto error;
    }
    len = 1;
    if (data) {
      *((uint8_t *)data) = head;
    } else {
      up->pos -= read;
    }
    goto exit;
  }

  has_len = false;
  endian = false;
  switch (head) {
    case FIXMAP_TAG: goto map_break;
    case FIXARRAY_TAG: goto arr_break;
    case FIXSTR_TAG: goto str_break;
    case BIN8_TAG: len_size = 1; goto bin_break;
    case BIN16_TAG: len_size = 2; goto bin_break;
    case BIN32_TAG: len_size = 4; goto bin_break;
bin_break:
      has_len = true;
      m_type = MSGPACK_TYPE_BIN;
      break;
    // case EXT8_TAG:
    // case EXT16_TAG:
    // case EXT32_TAG:
    case FLOAT_TAG: m_type = MSGPACK_TYPE_SINGLE; len = 4; goto endian_break;
    case DOUBLE_TAG: m_type = MSGPACK_TYPE_DOUBLE; len = 8; goto endian_break;
    case U8_TAG: m_type = MSGPACK_TYPE_U8; len = 1; goto endian_break;
    case U16_TAG: m_type = MSGPACK_TYPE_U16; len = 2; goto endian_break;
    case U32_TAG: m_type = MSGPACK_TYPE_U32; len = 4; goto endian_break;
    case U64_TAG: m_type = MSGPACK_TYPE_U64; len = 8; goto endian_break;
    case S8_TAG: m_type = MSGPACK_TYPE_S8; len = 1; goto endian_break;
    case S16_TAG: m_type = MSGPACK_TYPE_S16; len = 2; goto endian_break;
    case S32_TAG: m_type = MSGPACK_TYPE_S32; len = 4; goto endian_break;
    case S64_TAG: m_type = MSGPACK_TYPE_S64; len = 8; goto endian_break;
endian_break:
      endian = true;
      break;
    // case FIXEXT1_TAG:break;
    // case FIXEXT2_TAG:break;
    // case FIXEXT4_TAG:break;
    // case FIXEXT8_TAG:break;
    // case FIXEXT16_TAG:break;
    case STR8_TAG: has_len = true; len_size = 1; goto str_break;
    case STR16_TAG: has_len = true; len_size = 2; goto str_break;
    case STR32_TAG: has_len = true; len_size = 4; goto str_break;
str_break:
      m_type = MSGPACK_TYPE_STR;
      break;
    case ARRAY16_TAG: has_len = true; len_size = 2; goto arr_break;
    case ARRAY32_TAG: has_len = true; len_size = 4; goto arr_break;
arr_break:
      len_ok = true;
      m_type = MSGPACK_TYPE_ARRAY;
      break;
    case MAP16_TAG: has_len = true; len_size = 2; goto map_break;
    case MAP32_TAG: has_len = true; len_size = 4; goto map_break;
map_break:
      len_ok = true;
      m_type = MSGPACK_TYPE_MAP;
      break;
    default: set_errno(MSGPACK_EUNKNOWN); goto error;
  }

  if (*type != MSGPACK_TYPE_ANY && type_mask(*type) != type_mask(m_type)) {
    set_errno(MSGPACK_EUNEXPECTED);
    goto error;
  }

  if (has_len) {
    if (!msgpack_read_len(up, &len, len_size)) {
      goto error;
    }
    read += len_size;
  }

  /* Only read length, case as array and map type. */
  if (len_ok) {
    goto exit;
  }

  /* Just return data len. */
  if (!data) {
    up->pos -= read;
    goto exit;
  }

  if (*data_len < len) {
    set_errno(MSGPACK_ENOBUF);
    goto error;
  }

  if (endian) {
    ret = msgpack_read_endian(up, data, len);
  } else {
    ret = msgpack_read(up, data, len);
  }
  if(!ret) {
    goto error;
  }
  read += len;

exit:
  *type = m_type;
  if (data_len) {
    *data_len = len;
  }
  return true;

error:
  if(up) {
    up->pos -= read;
  }
  return false;
}

