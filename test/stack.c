#include "cutils/stack.h"
#include "test.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_stack_create (void);
static void test_stack_push_pop (void);
static void test_stack_peek (void);
static void test_stack_clear (void);
static void test_stack_is_empty (void);

void
run_stack_tests (void)
{
  test_stack_create ();
  test_stack_push_pop ();
  test_stack_peek ();
  test_stack_clear ();
  test_stack_is_empty ();
}

static void
test_stack_create (void)
{
  // test normal creation
  stack_t *stack = stack_create (4, sizeof (int32_t));
  assert (stack != NULL);
  assert (stack->top == 0);
  assert (stack->capacity == 4);
  assert (stack->elem_len == sizeof (int32_t));
  stack_destroy (stack);

  // test zero capacity (should use default)
  stack = stack_create (0, sizeof (int32_t));
  assert (stack != NULL);
  assert (stack->capacity == STACK_DEFAULT_CAPACITY);
  stack_destroy (stack);

  // test zero element size
  stack = stack_create (4, 0);
  assert (stack == NULL);
  assert (stack_get_error () == STACK_INVALID_ARG);

  // test overflow
  stack = stack_create (SIZE_MAX, sizeof (int32_t));
  assert (stack == NULL);
  assert (stack_get_error () == STACK_OUT_OF_RANGE);

  TEST_PASS ("stack_create");
}

static void
test_stack_push_pop (void)
{
  stack_t *stack = stack_create (4, sizeof (int32_t));
  int32_t values[] = { 1, 2, 3, 4 };
  int32_t val;

  // test pushing
  for (int32_t i = 0; i < 4; i++)
    {
      assert (stack_push (stack, &values[i]) == true);
    }
  assert (stack->top == 4);

  // test stack full
  assert (stack_push (stack, &values[0]) == false);
  assert (stack_get_error () == STACK_OUT_OF_RANGE);

  // test popping
  for (int32_t i = 3; i >= 0; i--)
    {
      assert (stack_pop (stack, &val) == true);
      assert (val == values[i]);
    }
  assert (stack->top == 0);

  // test pop empty stack
  assert (stack_pop (stack, &val) == false);
  assert (stack_get_error () == STACK_OUT_OF_RANGE);

  // test null parameters
  assert (stack_push (NULL, &val) == false);
  assert (stack_get_error () == STACK_NULL_PTR);
  assert (stack_push (stack, NULL) == false);
  assert (stack_get_error () == STACK_NULL_PTR);
  assert (stack_pop (NULL, &val) == false);
  assert (stack_get_error () == STACK_NULL_PTR);
  assert (stack_pop (stack, NULL) == false);
  assert (stack_get_error () == STACK_NULL_PTR);

  stack_destroy (stack);
  TEST_PASS ("stack_push_pop");
}

static void
test_stack_peek (void)
{
  stack_t *stack = stack_create (4, sizeof (int32_t));
  int32_t values[] = { 1, 2, 3 };
  int32_t val;

  // test empty stack
  assert (stack_peek (stack, &val) == false);
  assert (stack_get_error () == STACK_OUT_OF_RANGE);

  for (int32_t i = 0; i < 3; i++)
    {
      stack_push (stack, &values[i]);
    }

  // test normal case
  assert (stack_peek (stack, &val) == true);
  assert (val == values[2]);
  assert (stack->top == 3); // length shouldn't change

  // test null parameters
  assert (stack_peek (NULL, &val) == false);
  assert (stack_get_error () == STACK_NULL_PTR);
  assert (stack_peek (stack, NULL) == false);
  assert (stack_get_error () == STACK_NULL_PTR);

  stack_destroy (stack);
  TEST_PASS ("stack_peek");
}

static void
test_stack_clear (void)
{
  stack_t *stack = stack_create (4, sizeof (int32_t));
  int32_t values[] = { 1, 2, 3, 4 };

  for (int32_t i = 0; i < 4; i++)
    {
      stack_push (stack, &values[i]);
    }

  assert (stack_clear (stack) == true);
  assert (stack->top == 0);
  assert (stack->capacity == 4); // capacity shouldn't change

  stack_destroy (stack);

  // test null parameter
  assert (stack_clear (NULL) == false);
  assert (stack_get_error () == STACK_NULL_PTR);

  TEST_PASS ("stack_clear");
}

static void
test_stack_is_empty (void)
{
  stack_t *stack = stack_create (4, sizeof (int32_t));
  assert (stack_is_empty (stack) == true);

  int32_t val = 1;
  stack_push (stack, &val);
  assert (stack_is_empty (stack) == false);

  stack_pop (stack, &val);
  assert (stack_is_empty (stack) == true);

  // test null parameter
  assert (stack_is_empty (NULL) == true);
  assert (stack_get_error () == STACK_NULL_PTR);

  stack_destroy (stack);
  TEST_PASS ("stack_is_empty");
}
