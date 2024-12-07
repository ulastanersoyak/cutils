#include "cutils/stack.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

static thread_local stack_result_t g_last_error = STACK_OK;

stack_result_t
stack_get_error (void)
{
  return g_last_error;
}

stack_t *
stack_create (size_t capacity, size_t elem_len)
{
  g_last_error = STACK_OK;

  if (elem_len == 0)
    {
      g_last_error = STACK_INVALID_ARG;
      return NULL;
    }

  if (capacity == 0)
    {
      capacity = STACK_DEFAULT_CAPACITY;
    }

  if (SIZE_MAX / elem_len < capacity)
    {
      g_last_error = STACK_OUT_OF_RANGE;
      return NULL;
    }

  stack_t *stack = malloc (sizeof (stack_t));
  if (!stack)
    {
      g_last_error = STACK_NULL_PTR;
      return NULL;
    }

  stack->data = malloc (capacity * elem_len);
  if (!stack->data)
    {
      free (stack);
      g_last_error = STACK_NULL_PTR;
      return NULL;
    }

  stack->top = 0;
  stack->elem_len = elem_len;
  stack->capacity = capacity;
  return stack;
}

void
stack_destroy (stack_t *stack)
{
  g_last_error = STACK_OK;

  if (!stack)
    {
      g_last_error = STACK_NULL_PTR;
      return;
    }

  free (stack->data);
  free (stack);
}

bool
stack_push (stack_t *stack, const void *elem)
{
  g_last_error = STACK_OK;

  if (!stack || !elem)
    {
      g_last_error = STACK_NULL_PTR;
      return false;
    }

  if (stack->top >= stack->capacity)
    {
      g_last_error = STACK_OUT_OF_RANGE;
      return false;
    }

  memcpy ((char *)stack->data + (stack->top * stack->elem_len), elem,
          stack->elem_len);
  stack->top++;
  return true;
}

bool
stack_pop (stack_t *stack, void *out)
{
  g_last_error = STACK_OK;

  if (!stack || !out)
    {
      g_last_error = STACK_NULL_PTR;
      return false;
    }

  if (stack->top == 0)
    {
      g_last_error = STACK_OUT_OF_RANGE;
      return false;
    }

  stack->top--;
  memcpy (out, (char *)stack->data + (stack->top * stack->elem_len),
          stack->elem_len);
  return true;
}

bool
stack_peek (const stack_t *stack, void *out)
{
  g_last_error = STACK_OK;

  if (!stack || !out)
    {
      g_last_error = STACK_NULL_PTR;
      return false;
    }

  if (stack->top == 0)
    {
      g_last_error = STACK_OUT_OF_RANGE;
      return false;
    }

  memcpy (out, (char *)stack->data + ((stack->top - 1) * stack->elem_len),
          stack->elem_len);
  return true;
}

bool
stack_clear (stack_t *stack)
{
  g_last_error = STACK_OK;

  if (!stack)
    {
      g_last_error = STACK_NULL_PTR;
      return false;
    }

  stack->top = 0;
  return true;
}

bool
stack_is_empty (const stack_t *stack)
{
  g_last_error = STACK_OK;

  if (!stack)
    {
      g_last_error = STACK_NULL_PTR;
      return true;
    }

  return stack->top == 0;
}
