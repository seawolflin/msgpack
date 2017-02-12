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
#include <stdlib.h>
#include <stdio.h>

#define test_log printf
#define TEST_PRINT(format, ...) test_log(format "\n", __VA_ARGS__)

#define EXPECT_COND(condition) \
   if (!(condition)) { \
    TEST_PRINT("[TEST] EXPECT FAILED: %s ", #condition); \
  }
#define EXPECT_OP(val1, val2, op) EXPECT_COND((val1) op (val2))

#define EXPECT_EQ(expected, actual) EXPECT_OP(expected, actual, ==)
#define EXPECT_NE(expected, actual) EXPECT_OP(expected, actual, !=)
#define EXPECT_LE(val1, val2) EXPECT_OP(val1, val2, <=)
#define EXPECT_LT(val1, val2) EXPECT_OP(val1, val2, <)
#define EXPECT_GE(val1, val2) EXPECT_OP(val1, val2, >=)
#define EXPECT_GT(val1, val2) EXPECT_OP(val1, val2, >)

#define EXPECT_TRUE(condition)  EXPECT_COND(condition)
#define EXPECT_FALSE(condition) EXPECT_COND(!(condition))

#define ASSERT_COND(condition) \
   if (!(condition)) { \
    TEST_PRINT("[TEST] ASSERT FAILED: %s ", #condition); \
    abort(); \
  }
#define ASSERT_OP(val1, val2, op) ASSERT_COND((val1) op (val2))

# define ASSERT_EQ(val1, val2) ASSERT_OP(val1, val2, ==)
# define ASSERT_NE(val1, val2) ASSERT_OP(val1, val2, !=)
# define ASSERT_LE(val1, val2) ASSERT_OP(val1, val2, <=)
# define ASSERT_LT(val1, val2) ASSERT_OP(val1, val2, >=)
# define ASSERT_GE(val1, val2) ASSERT_OP(val1, val2, >)

#define ASSERT_TRUE(condition)  ASSERT_COND(condition)
#define ASSERT_FALSE(condition) ASSERT_COND(!(condition))


#define TEST(test_suite, test_case) \
  void test_##test_suite## _ ##test_case()

#define DECLARE_TEST(test_suite, test_case) \
  void test_##test_suite## _ ##test_case()

#define RUN_TEST(test_suite, test_case) \
  do { \
    TEST_PRINT("[TEST] Running <<%s>>", "test_" #test_suite "_" #test_case); \
    test_##test_suite##_##test_case (); \
    TEST_PRINT("[TEST] End run <<%s>>", "test_" #test_suite "_" #test_case); \
  } while (0)


