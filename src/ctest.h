#ifndef UUID_b743105c6bd5471c871fa85d8ec31a1c
#define UUID_b743105c6bd5471c871fa85d8ec31a1c

/*
  CTEST -- A simple unit test framework for C
  Copyright (C) 2011, by Christopher A. Mosher

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



/*

CTEST is a very simple unit test framework for C.
Its main functions are to check for pass/fail of tests,
print error messages for failures, and keep count.

To use the CTEST framework, perform to following
actions:
  1. Call ctest_ctx_alloc to allocate a suite context.
  2. Call the CTEST macro for each unit test to perform.
  3. Check the count of failed tests in the suite by
     calling ctest_count_failures.
  4. Call ctest_ctx_free to free the suite context.

Here is a simple example:

#include "ctest.h"
int main(int argc, char **argv)
{
  int f;

  ctest_ctx *ctx = ctest_ctx_alloc();

  CTEST(ctx,9/3==3);
  CTEST(ctx,10/3==3);
  f = ctest_count_fail(ctx);

  ctest_ctx_free(ctx);

  if (f)
    {
      return 1;
    }

  return 0;
}

*/


#ifdef __cplusplus
extern "C" {
#endif





/*
  Opaque structure containing the CTEST context for
  one suite of unit tests.
*/
struct ctest_ctx;
typedef struct ctest_ctx ctest_ctx;



/*
  Functions to allocate and deallocate a CTEST context.
*/
extern ctest_ctx *ctest_ctx_alloc(void);
extern void ctest_ctx_free(ctest_ctx *ctx);



/*
  Performs one unit test. Call the CTEST macro for each unit
  test to perform. Use the CTEST macro, not the ctest
  function directly, because the CTEST macro gets the current
  file name and line number of the test (which it uses in the
  error message in case the test fails).
  ctx is the CTEST context, which must have been allocated by
    a call to ctest_ctx_alloc.
  assertion is the boolean expression to test. If the expression
    evaluates to non-zero (true), then the test is considered
    correct; if the expression evaluates to zero (false), then
    the test is considered failed.
*/
#define CTEST(ctx,assertion) ctest(ctx,#assertion,assertion,__FILE__,__LINE__)

extern void ctest(ctest_ctx *ctx, const char *name, int is_true, const char *file_name, const unsigned long line_number);



/*
  Accessor functions to get the (current) count of tests
  that passed or failed.
*/
extern long ctest_count_pass(const ctest_ctx *ctx);
extern long ctest_count_fail(const ctest_ctx *ctx);
extern long ctest_count_test(const ctest_ctx *ctx);





#ifdef __cplusplus
}
#endif

#endif
