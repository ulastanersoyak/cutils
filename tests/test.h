#ifndef CUTILS_TEST_H
#define CUTILS_TEST_H

/* Minimal test helpers. Each test file is its own program so a crash in one
   module can't hide failures in another. */

#include <stdio.h>

static int test_failures = 0;

#define CHECK(cond)                                                           \
  do                                                                          \
    {                                                                         \
      if (!(cond))                                                            \
        {                                                                     \
          printf ("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);             \
          test_failures++;                                                    \
        }                                                                     \
    }                                                                         \
  while (0)

/* Overwrites a chunk of stack, so a container that kept a pointer into a
   dead stack frame reads garbage instead of passing by luck. */
[[maybe_unused]] static void __attribute__ ((noinline))
clobber_stack (void)
{
  volatile char junk[4096];
  for (size_t i = 0; i < sizeof junk; i++)
    {
      junk[i] = (char)0xA5;
    }
}

[[maybe_unused]] static int
compare_ints (const void *a, const void *b)
{
  int x = *(const int *)a;
  int y = *(const int *)b;
  return (x > y) - (x < y);
}

static inline int
test_result (const char *name)
{
  printf ("%s: %s\n", name, test_failures == 0 ? "pass" : "FAIL");
  return test_failures == 0 ? 0 : 1;
}

#endif /* CUTILS_TEST_H */
