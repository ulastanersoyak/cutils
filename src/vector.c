#include "cutils/vector.h"
#include "cutils/config.h"
#include "cutils/time.h"
#include <string.h>

#include <stdint.h>
#include <stdlib.h>
#include <threads.h>

static thread_local vector_result_t g_last_error = VECTOR_OK;

static bool
check_timeout ([[maybe_unused]] uint32_t start_time_ms,
               [[maybe_unused]] uint32_t timeout_ms)
{
#if CUTILS_PLATFORM_BARE_METAL
  // Implement platform-specific time check
  return true;
#else
  uint32_t current_time = get_current_time_ms ();
  return (current_time - start_time_ms) <= timeout_ms;
#endif
}

vector_t *
vector_create_with_allocator (size_t init_capacity, size_t elem_len,
                              cutils_allocator_t *allocator)
{
  g_last_error = VECTOR_OK;

  if (elem_len == 0 || allocator == NULL)
    {
      g_last_error = VECTOR_INVALID_ARG;
      return NULL;
    }

  if (SIZE_MAX / elem_len < init_capacity)
    {
      g_last_error = VECTOR_OVERFLOW;
      return NULL;
    }

  if (init_capacity == 0)
    {
      init_capacity = CUTILS_VECTOR_INIT_CAPACITY;
    }

  vector_t *vec = cutils_allocate_aligned (allocator, sizeof (vector_t),
                                           CUTILS_ALIGNMENT);
  if (vec == NULL)
    {
      g_last_error = VECTOR_NO_MEMORY;
      return NULL;
    }

  vec->data = cutils_allocate_aligned (allocator, init_capacity * elem_len,
                                       CUTILS_ALIGNMENT);
  if (vec->data == NULL)
    {
      cutils_deallocate (allocator, vec);
      g_last_error = VECTOR_NO_MEMORY;
      return NULL;
    }

  vec->len = 0;
  vec->capacity = init_capacity;
  vec->elem_len = elem_len;
  vec->allocator = allocator;

  return vec;
}

vector_t *
vector_create (size_t init_capacity, size_t elem_len)
{
  cutils_allocator_t default_allocator = cutils_create_default_allocator ();
  return vector_create_with_allocator (init_capacity, elem_len,
                                       &default_allocator);
}

static bool
vector_push_timeout (vector_t *vec, const void *elem, uint32_t timeout_ms)
{
  g_last_error = VECTOR_OK;

  if (vec == NULL || elem == NULL)
    {
      g_last_error = VECTOR_NULL_PTR;
      return false;
    }

  uint64_t start_time = cutils_get_current_time_ms ();

  if (vec->len >= vec->capacity)
    {
      size_t new_capacity = vec->capacity == 0
                                ? CUTILS_VECTOR_INIT_CAPACITY
                                : vec->capacity * CUTILS_VECTOR_GROWTH_FACTOR;

      if (new_capacity > CUTILS_VECTOR_MAX_CAPACITY)
        {
          g_last_error = VECTOR_OVERFLOW;
          return false;
        }

      if (!check_timeout ((uint32_t)start_time, timeout_ms))
        {
          g_last_error = VECTOR_TIMEOUT;
          return false;
        }

      void *new_data = cutils_allocate_aligned (
          vec->allocator, new_capacity * vec->elem_len, CUTILS_ALIGNMENT);
      if (new_data == NULL)
        {
          g_last_error = VECTOR_NO_MEMORY;
          return false;
        }

      memcpy (new_data, vec->data, vec->len * vec->elem_len);
      cutils_deallocate (vec->allocator, vec->data);
      vec->data = new_data;
      vec->capacity = new_capacity;
    }

  memcpy ((char *)vec->data + (vec->len * vec->elem_len), elem, vec->elem_len);
  vec->len++;

  return true;
}

bool
vector_push (vector_t *vec, const void *elem)
{
  return vector_push_timeout (vec, elem, CUTILS_MAX_OPERATION_TIME_MS);
}

[[nodiscard]] vector_result_t
vector_get_error (void)
{
  return g_last_error;
}

[[nodiscard]] vector_t *
vector_copy (const vector_t *vec)
{
  g_last_error = VECTOR_OK;

  if (vec == NULL)
    {
      g_last_error = VECTOR_NULL_PTR;
      return NULL;
    }

  vector_t *copy_vec = malloc (sizeof (vector_t));
  if (copy_vec == NULL)
    {
      g_last_error = VECTOR_NO_MEMORY;
      return NULL;
    }

  copy_vec->data = malloc (vec->elem_len * vec->capacity);
  if (copy_vec->data == NULL)
    {
      free (copy_vec);
      g_last_error = VECTOR_NO_MEMORY;
      return NULL;
    }

  memcpy (copy_vec->data, vec->data, vec->elem_len * vec->len);
  copy_vec->capacity = vec->capacity;
  copy_vec->len = vec->len;
  copy_vec->elem_len = vec->elem_len;

  return copy_vec;
}

void
vector_destroy (vector_t *vec)
{
  g_last_error = VECTOR_OK;

  if (vec == NULL)
    {
      g_last_error = VECTOR_NULL_PTR;
    }

  if (vec->data != NULL)
    {
      free (vec->data);
    }

  free (vec);
}

bool
vector_pop (vector_t *vec, void *out)
{
  g_last_error = VECTOR_OK;

  if (vec == NULL)
    {
      g_last_error = VECTOR_NULL_PTR;
      return false;
    }

  if (vec->capacity == 0 || vec->len == 0)
    {
      g_last_error = VECTOR_OUT_OF_RANGE;
      return false;
    }

  memcpy (out, (char *)vec->data + ((vec->len - 1) * vec->elem_len),
          vec->elem_len);
  vec->len--;

  return true;
}

bool
vector_get (const vector_t *vec, size_t index, void *out)
{
  g_last_error = VECTOR_OK;

  if (vec == NULL)
    {
      g_last_error = VECTOR_NULL_PTR;
      return false;
    }

  if (index >= vec->len)
    {
      g_last_error = VECTOR_OUT_OF_RANGE;
      return false;
    }

  memcpy (out, (char *)vec->data + (index * vec->elem_len), vec->elem_len);

  return true;
}

bool
vector_set (vector_t *vec, size_t index, const void *elem)
{
  g_last_error = VECTOR_OK;

  if (vec == NULL || elem == NULL)
    {
      g_last_error = VECTOR_NULL_PTR;
      return false;
    }

  if (index >= vec->len)
    {
      g_last_error = VECTOR_OUT_OF_RANGE;
      return false;
    }

  memcpy ((char *)vec->data + (index * vec->elem_len), elem, vec->elem_len);
  return true;
}

bool
vector_insert (vector_t *vec, size_t index, const void *elem)
{
  g_last_error = VECTOR_OK;

  if (vec == NULL || elem == NULL)
    {
      g_last_error = VECTOR_NULL_PTR;
      return false;
    }

  if (index > vec->len)
    {
      g_last_error = VECTOR_OUT_OF_RANGE;
      return false;
    }

  if (vec->len >= vec->capacity)
    {
      size_t new_capacity = vec->capacity == 0
                                ? CUTILS_VECTOR_INIT_CAPACITY
                                : vec->capacity * CUTILS_VECTOR_GROWTH_FACTOR;

      if (SIZE_MAX / vec->elem_len < new_capacity)
        {
          g_last_error = VECTOR_OVERFLOW;
          return false;
        }

      void *new_data = realloc (vec->data, new_capacity * vec->elem_len);
      if (new_data == NULL)
        {
          g_last_error = VECTOR_NO_MEMORY;
          return false;
        }

      vec->data = new_data;
      vec->capacity = new_capacity;
    }

  if (index < vec->len)
    {
      memmove ((char *)vec->data + ((index + 1) * vec->elem_len),
               (char *)vec->data + (index * vec->elem_len),
               (vec->len - index) * vec->elem_len);
    }

  memcpy ((char *)vec->data + (index * vec->elem_len), elem, vec->elem_len);
  vec->len++;

  return true;
}

bool
vector_remove (vector_t *vec, size_t index, void *out)
{
  g_last_error = VECTOR_OK;

  if (vec == NULL)
    {
      g_last_error = VECTOR_NULL_PTR;
      return false;
    }

  if (index >= vec->len)
    {
      g_last_error = VECTOR_OUT_OF_RANGE;
      return false;
    }

  if (out != NULL)
    {
      memcpy (out, (char *)vec->data + (index * vec->elem_len), vec->elem_len);
    }

  if (index < vec->len - 1)
    {
      memmove ((char *)vec->data + (index * vec->elem_len),
               (char *)vec->data + ((index + 1) * vec->elem_len),
               (vec->len - index - 1) * vec->elem_len);
    }

  vec->len--;
  return true;
}

bool
vector_reserve (vector_t *vec, size_t capacity)
{
  g_last_error = VECTOR_OK;

  if (vec == NULL)
    {
      g_last_error = VECTOR_NULL_PTR;
      return false;
    }

  if (capacity <= vec->capacity)
    {
      return true;
    }

  if (SIZE_MAX / vec->elem_len < capacity)
    {
      g_last_error = VECTOR_OVERFLOW;
      return false;
    }

  void *new_data = realloc (vec->data, capacity * vec->elem_len);
  if (new_data == NULL)
    {
      g_last_error = VECTOR_NO_MEMORY;
      return false;
    }

  vec->data = new_data;
  vec->capacity = capacity;

  return true;
}

bool
vector_shrink (vector_t *vec)
{
  g_last_error = VECTOR_OK;

  if (vec == NULL)
    {
      g_last_error = VECTOR_NULL_PTR;
      return false;
    }

  if (vec->len == vec->capacity)
    {
      return true;
    }

  if (vec->len == 0)
    {
      free (vec->data);
      vec->data = NULL;
      vec->capacity = 0;
      return true;
    }

  void *new_data = realloc (vec->data, vec->len * vec->elem_len);
  if (new_data == NULL)
    {
      g_last_error = VECTOR_NO_MEMORY;
      return false;
    }

  vec->data = new_data;
  vec->capacity = vec->len;

  return true;
}

bool
vector_clear (vector_t *vec)
{
  g_last_error = VECTOR_OK;

  if (vec == NULL)
    {
      g_last_error = VECTOR_NULL_PTR;
      return false;
    }

  vec->len = 0;
  return true;
}

bool
vector_is_empty (const vector_t *vec)
{
  g_last_error = VECTOR_OK;

  if (vec == NULL)
    {
      g_last_error = VECTOR_NULL_PTR;
      return true; // return true for NULL as it's effectively empty
    }

  return vec->len == 0;
}

bool
vector_back (const vector_t *vec, void *out)
{
  g_last_error = VECTOR_OK;

  if (vec == NULL || out == NULL)
    {
      g_last_error = VECTOR_NULL_PTR;
      return false;
    }

  if (vec->len == 0)
    {
      g_last_error = VECTOR_OUT_OF_RANGE;
      return false;
    }

  memcpy (out, (char *)vec->data + ((vec->len - 1) * vec->elem_len),
          vec->elem_len);
  return true;
}

size_t
vector_memory_usage (const vector_t *vec)
{
  if (vec == NULL)
    {
      g_last_error = VECTOR_NULL_PTR;
      return 0;
    }
  return vec->capacity * vec->elem_len + sizeof (vector_t);
}

bool
vector_can_perform_operation (const vector_t *vec, size_t required_capacity)
{
  if (vec == NULL)
    {
      g_last_error = VECTOR_NULL_PTR;
      return false;
    }

  if (required_capacity <= vec->capacity)
    {
      return true;
    }

  size_t new_capacity = vec->capacity == 0
                            ? CUTILS_VECTOR_INIT_CAPACITY
                            : vec->capacity * CUTILS_VECTOR_GROWTH_FACTOR;

  if (new_capacity > CUTILS_VECTOR_MAX_CAPACITY)
    {
      g_last_error = VECTOR_OVERFLOW;
      return false;
    }

  return cutils_can_allocate (vec->allocator, new_capacity * vec->elem_len,
                              CUTILS_ALIGNMENT);
}
