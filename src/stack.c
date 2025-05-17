#include "cutils/stack.h"
#include "capacity.h"
#include "cutils/config.h"
#include "cutils/time.h"
#include <stdint.h>
#include <string.h>
#include <threads.h>

static thread_local stack_result_t g_last_error = STACK_OK;

stack_t *
stack_create_with_allocator (size_t elem_size, size_t initial_capacity,
                             cutils_allocator_t *allocator)
{
  g_last_error = STACK_OK;

  if (elem_size == 0 || allocator == NULL)
    {
      g_last_error = STACK_INVALID_ARG;
      return NULL;
    }

  size_t bytes;
  if (!cutils_array_bytes (initial_capacity, elem_size, &bytes))
    {
      g_last_error = STACK_OVERFLOW;
      return NULL;
    }

  stack_t *stack = cutils_allocate_aligned (allocator, sizeof (stack_t),
                                            CUTILS_ALIGNMENT);
  if (stack == NULL)
    {
      g_last_error = STACK_NO_MEMORY;
      return NULL;
    }

  // with capacity 0 the buffer is allocated on the first push
  stack->data = NULL;
  if (bytes > 0)
    {
      stack->data
          = cutils_allocate_aligned (allocator, bytes, CUTILS_ALIGNMENT);
      if (stack->data == NULL)
        {
          cutils_deallocate (allocator, stack);
          g_last_error = STACK_NO_MEMORY;
          return NULL;
        }
    }

  stack->size = 0;
  stack->capacity = initial_capacity;
  stack->elem_size = elem_size;
  stack->allocator = allocator;

  return stack;
}

stack_t *
stack_create (size_t elem_size, size_t initial_capacity)
{
  return stack_create_with_allocator (elem_size, initial_capacity,
                                      cutils_default_allocator ());
}

void
stack_destroy (stack_t *stack)
{
  g_last_error = STACK_OK;

  if (stack == NULL)
    {
      g_last_error = STACK_NULL_PTR;
      return;
    }

  cutils_deallocate (stack->allocator, stack->data);
  cutils_deallocate (stack->allocator, stack);
}

static bool
resize_if_needed (stack_t *stack, uint32_t timeout_ms)
{
  if (stack->size < stack->capacity)
    {
      return true;
    }

  uint64_t start_time = cutils_get_current_time_ms ();

  size_t new_capacity;
  size_t bytes;
  if (!cutils_next_capacity (stack->capacity, stack->size + 1, SIZE_MAX,
                             &new_capacity)
      || !cutils_array_bytes (new_capacity, stack->elem_size, &bytes))
    {
      g_last_error = STACK_OVERFLOW;
      return false;
    }

  void *new_data = cutils_reallocate (stack->allocator, stack->data,
                                      stack->size * stack->elem_size, bytes,
                                      CUTILS_ALIGNMENT);
  if (new_data == NULL)
    {
      g_last_error = STACK_NO_MEMORY;
      return false;
    }

  stack->data = new_data;
  stack->capacity = new_capacity;

  if (cutils_timeout_expired (start_time, timeout_ms))
    {
      g_last_error = STACK_TIMEOUT;
      return false;
    }

  return true;
}

bool
stack_push_timeout (stack_t *stack, const void *elem, uint32_t timeout_ms)
{
  g_last_error = STACK_OK;

  if (stack == NULL || elem == NULL)
    {
      g_last_error = STACK_NULL_PTR;
      return false;
    }

  if (!resize_if_needed (stack, timeout_ms))
    {
      return false;
    }

  memcpy ((char *)stack->data + (stack->size * stack->elem_size), elem,
          stack->elem_size);
  stack->size++;

  return true;
}

bool
stack_push (stack_t *stack, const void *elem)
{
  return stack_push_timeout (stack, elem, CUTILS_MAX_OPERATION_TIME_MS);
}

bool
stack_pop (stack_t *stack, void *out_elem)
{
  g_last_error = STACK_OK;

  if (stack == NULL || out_elem == NULL)
    {
      g_last_error = STACK_NULL_PTR;
      return false;
    }

  if (stack->size == 0)
    {
      g_last_error = STACK_EMPTY;
      return false;
    }

  stack->size--;
  memcpy (out_elem, (char *)stack->data + (stack->size * stack->elem_size),
          stack->elem_size);

  return true;
}

bool
stack_peek (const stack_t *stack, void *out_elem)
{
  g_last_error = STACK_OK;

  if (stack == NULL || out_elem == NULL)
    {
      g_last_error = STACK_NULL_PTR;
      return false;
    }

  if (stack->size == 0)
    {
      g_last_error = STACK_EMPTY;
      return false;
    }

  memcpy (out_elem,
          (char *)stack->data + ((stack->size - 1) * stack->elem_size),
          stack->elem_size);

  return true;
}

size_t
stack_size (const stack_t *stack)
{
  if (stack == NULL)
    {
      g_last_error = STACK_NULL_PTR;
      return 0;
    }
  return stack->size;
}

size_t
stack_capacity (const stack_t *stack)
{
  if (stack == NULL)
    {
      g_last_error = STACK_NULL_PTR;
      return 0;
    }
  return stack->capacity;
}

bool
stack_is_empty (const stack_t *stack)
{
  g_last_error = STACK_OK;

  if (stack == NULL)
    {
      g_last_error = STACK_NULL_PTR;
      return true;
    }

  return stack->size == 0;
}

bool
stack_is_full (const stack_t *stack)
{
  g_last_error = STACK_OK;

  if (stack == NULL)
    {
      g_last_error = STACK_NULL_PTR;
      return true;
    }

  return stack->size == stack->capacity;
}

bool
stack_clear (stack_t *stack)
{
  g_last_error = STACK_OK;

  if (stack == NULL)
    {
      g_last_error = STACK_NULL_PTR;
      return false;
    }

  stack->size = 0;
  return true;
}

size_t
stack_memory_usage (const stack_t *stack)
{
  if (stack == NULL)
    {
      g_last_error = STACK_NULL_PTR;
      return 0;
    }
  return sizeof (stack_t) + (stack->capacity * stack->elem_size);
}

bool
stack_can_perform_operation (const stack_t *stack, size_t required_capacity)
{
  if (stack == NULL)
    {
      g_last_error = STACK_NULL_PTR;
      return false;
    }

  return cutils_can_allocate (stack->allocator, required_capacity,
                              CUTILS_ALIGNMENT);
}

stack_result_t
stack_get_error (void)
{
  return g_last_error;
}
