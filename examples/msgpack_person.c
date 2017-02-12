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
#include <stdio.h>
#include "msgpack.h"

void msgpack_person(void) {
  msgpack_buffer_t mbuf;
  int8_t data[64];

  init_msgpack_buffer(&mbuf, data, 64);
  /* Encode map ["name":"Joan", "age": 30, "gender", "female"] */
  msgpack_write_map(&mbuf, 3);
  msgpack_write_str(&mbuf, "name", strlen("name"));
  msgpack_write_str(&mbuf, "Joan", strlen("Joan"));
  msgpack_write_str(&mbuf, "age", strlen("age"));
  msgpack_write_integer(&mbuf, 30);
  msgpack_write_str(&mbuf, "gender", strlen("gender"));
  msgpack_write_str(&mbuf, "female", strlen("female"));

  msgpack_unpacker_t unpacker;
  uint8_t type;
  uint32_t len;
  uint32_t map_len;
  char key[8] = {0};
  char strval[8] = {0};
  int age = 0;
  int i;
  init_msgpack_unpacker(&unpacker, mbuf.buf, mbuf.len, 0);
  type = MSGPACK_TYPE_ANY;
  msgpack_unpack(&unpacker, NULL, &map_len, &type);
  if (msgpack_errno() == MSGPACK_EOK) {
    printf("map len: %d \n[", map_len);
  }

  for (i = 0; i < map_len; ++i) {
    type = MSGPACK_TYPE_ANY;
    len = 8;
    msgpack_unpack(&unpacker, &key, &len, &type);
    if (msgpack_errno() == MSGPACK_EOK) {
      key[len]=0;
      printf("\"%s\":", key);
    }
    type = MSGPACK_TYPE_ANY;
    if (i == 1) {
      len = 4;
      msgpack_unpack(&unpacker, &age, &len, &type);
      if (msgpack_errno() == MSGPACK_EOK) {
        printf("%d", age);
      }
    } else {
      len = 8;
      msgpack_unpack(&unpacker, &strval, &len, &type);
      if (msgpack_errno() == MSGPACK_EOK) {
        strval[len] = 0;
        printf("\"%s\"", strval);
      }
    }
    if (i < map_len - 1) {
      printf(", ");
    }
  }
  printf("]\n");
}

int main(void) {
  msgpack_person();
  return 0;
}

