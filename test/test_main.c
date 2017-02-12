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

#include "msgpack.h"
#include "test.h"

DECLARE_TEST(msgpack, write_simple);
DECLARE_TEST(msgpack, write_string);
DECLARE_TEST(msgpack, read_simple);
DECLARE_TEST(msgpack, read_string);
DECLARE_TEST(msgpack, read_len);
DECLARE_TEST(msgpack, error_call);

int main(void) {
  RUN_TEST(msgpack, write_simple);
  RUN_TEST(msgpack, write_string);
  RUN_TEST(msgpack, read_simple);
  RUN_TEST(msgpack, read_string);
  RUN_TEST(msgpack, read_len);
  RUN_TEST(msgpack, error_call);
  return 0;
}
