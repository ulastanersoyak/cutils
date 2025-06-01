#include "cutils/vector.h"
#include "capacity.h"
#include "cutils/config.h"
#include "cutils/time.h"
#include <string.h>

#include <stdint.h>
#include <threads.h>

static thread_local vector_result_t g_last_error = VECTOR_OK;

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

  if (init_capacity == 0)
    {
      init_capacity = CUTILS_VECTOR_INIT_CAPACITY;
    }

  size_t bytes;
  if (init_capacity > CUTILS_VECTOR_MAX_CAPACITY
      || !cutils_array_bytes (init_capacity, elem_len, &bytes))
    {
      g_last_error = VECTOR_OVERFLOW;
      return NULL;
    }

  vector_t *vec = cutils_allocate_aligned (allocator, sizeof (vector_t),
                                           CUTILS_ALIGNMENT);
  if (vec == NULL)
    {
      g_last_error = VECTOR_NO_MEMORY;
      return NULL;
    }

  vec->data = cutils_allocate_aligned (allocator, bytes, CUTILS_ALIGNMENT);
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

// moves the elements into a buffer of exactly capacity elements, through the
// vector's own allocator. capacity must be at least vec->len
static bool
vector_set_capacity (vector_t *vec, size_t capacity)
{
  size_t bytes;
  if (capacity > CUTILS_VECTOR_MAX_CAPACITY
      || !cutils_array_bytes (capacity, vec->elem_len, &bytes))
    {
      g_last_error = VECTOR_OVERFLOW;
      return false;
    }

  void *new_data
      = cutils_reallocate (vec->allocator, vec->data, vec->len * vec->elem_len,
                           bytes, CUTILS_ALIGNMENT);
  if (new_data == NULL)
    {
      g_last_error = VECTOR_NO_MEMORY;
      return false;
    }

  vec->data = new_data;
  vec->capacity = capacity;
  return true;
}

// makes room for at least required elements, growing geometrically
static bool
vector_grow (vector_t *vec, size_t required)
{
  if (required <= vec->capacity)
    {
      return true;
    }

  size_t new_capacity;
  if (!cutils_next_capacity (vec->capacity, required,
                             CUTILS_VECTOR_MAX_CAPACITY, &new_capacity))
    {
      g_last_error = VECTOR_OVERFLOW;
      return false;
    }

  return vector_set_capacity (vec, new_capacity);
}

vector_t *
vector_create (size_t init_capacity, size_t elem_len)
{
  return vector_create_with_allocator (init_capacity, elem_len,
                                       cutils_default_allocator ());
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

  if (vec->len == SIZE_MAX || !vector_grow (vec, vec->len + 1))
    {
      return false;
    }

  if (cutils_timeout_expired (start_time, timeout_ms))
    {
      g_last_error = VECTOR_TIMEOUT;
      return false;
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

  vector_t *copy_vec = vector_create_with_allocator (
      vec->capacity, vec->elem_len, vec->allocator);
  if (copy_vec == NULL)
    {
      return NULL;
    }

  if (vec->len > 0)
    {
      memcpy (copy_vec->data, vec->data, vec->elem_len * vec->len);
    }
  copy_vec->len = vec->len;

  return copy_vec;
}

void
vector_destroy (vector_t *vec)
{
  g_last_error = VECTOR_OK;

  if (vec == NULL)
    {
      g_last_error = VECTOR_NULL_PTR;
      return;
    }

  cutils_deallocate (vec->allocator, vec->data);
  cutils_deallocate (vec->allocator, vec);
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

  if (vec->len == 0)
    {
      g_last_error = VECTOR_OUT_OF_RANGE;
      return false;
    }

  if (out != NULL)
    {
      memcpy (out, (char *)vec->data + ((vec->len - 1) * vec->elem_len),
              vec->elem_len);
    }
  vec->len--;

  return true;
}

bool
vector_get (const vector_t *vec, size_t index, void *out)
{
  g_last_error = VECTOR_OK;

  if (vec == NULL || out == NULL)
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

  if (vec->len == SIZE_MAX || !vector_grow (vec, vec->len + 1))
    {
      return false;
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

  return vector_set_capacity (vec, capacity);
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
      cutils_deallocate (vec->allocator, vec->data);
      vec->data = NULL;
      vec->capacity = 0;
      return true;
    }

  return vector_set_capacity (vec, vec->len);
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

void *
vector_at (vector_t *vec, size_t index)
{
  g_last_error = VECTOR_OK;

  if (vec == NULL)
    {
      g_last_error = VECTOR_NULL_PTR;
      return NULL;
    }

  if (index >= vec->len)
    {
      g_last_error = VECTOR_OUT_OF_RANGE;
      return NULL;
    }

  return (char *)vec->data + (index * vec->elem_len);
}

void *
vector_data (vector_t *vec)
{
  if (vec == NULL)
    {
      g_last_error = VECTOR_NULL_PTR;
      return NULL;
    }
  return vec->data;
}

bool
vector_swap_remove (vector_t *vec, size_t index, void *out)
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

  char *slot = (char *)vec->data + (index * vec->elem_len);
  if (out != NULL)
    {
      memcpy (out, slot, vec->elem_len);
    }

  size_t last = vec->len - 1;
  if (index != last)
    {
      memcpy (slot, (char *)vec->data + (last * vec->elem_len), vec->elem_len);
    }

  vec->len--;
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

  size_t new_capacity;
  size_t bytes;
  if (!cutils_next_capacity (vec->capacity, required_capacity,
                             CUTILS_VECTOR_MAX_CAPACITY, &new_capacity)
      || !cutils_array_bytes (new_capacity, vec->elem_len, &bytes))
    {
      g_last_error = VECTOR_OVERFLOW;
      return false;
    }

  return cutils_can_allocate (vec->allocator, bytes, CUTILS_ALIGNMENT);
}

size_t
vector_length (const vector_t *vec)
{
  if (vec == NULL)
    {
      g_last_error = VECTOR_NULL_PTR;
      return 0;
    }
  return vec->len;
}

size_t
vector_capacity (const vector_t *vec)
{
  if (vec == NULL)
    {
      g_last_error = VECTOR_NULL_PTR;
      return 0;
    }
  return vec->capacity;
}

size_t
vector_element_size (const vector_t *vec)
{
  if (vec == NULL)
    {
      g_last_error = VECTOR_NULL_PTR;
      return 0;
    }
  return vec->elem_len;
}
