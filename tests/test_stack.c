#define _POSIX_C_SOURCE 200809L
#include "cutils/stack.h"
#include "test.h"
#include <signal.h>
#include <stdlib.h>
#include <string.h>

static void
test_stack (void)
{
  cutils_stack_t *s = stack_create (sizeof (int), 0);
  CHECK (s != NULL);
  if (!s)
    return;
  clobber_stack ();
  for (int i = 0; i < 50; i++)
    CHECK (stack_push (s, &i));
  int x;
  CHECK (stack_pop (s, &x) && x == 49);
  CHECK (stack_size (s) == 49);
  CHECK (stack_create (SIZE_MAX / 2, 4) == NULL);
  stack_destroy (s);
}

// posix stack_t (for sigaltstack) and the cutils stack must coexist
static void
test_signal_header (void)
{
  stack_t alternate = { 0 };
  cutils_stack_t *stack = stack_create (sizeof (int), 0);
  int x = 4;
  CHECK (stack_push (stack, &x) && stack_pop (stack, &x) && x == 4);
  CHECK (alternate.ss_size == 0);
  stack_destroy (stack);
}

int
main (void)
{
  test_stack ();
  test_signal_header ();
  return test_result ("stack");
}
