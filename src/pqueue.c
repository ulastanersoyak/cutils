#include "cutils/pqueue.h"
#include "cutils/config.h"
#include "cutils/time.h"
#include <stdint.h>
#include <string.h>
#include <threads.h>

static thread_local priority_queue_result_t g_last_error = PRIORITY_QUEUE_OK;

// Helper functions for binary heap operations
static size_t
parent (size_t i)
{
  return (i - 1) / 2;
}
static size_t
left_child (size_t i)
{
  return 2 * i + 1;
}
static size_t
right_child (size_t i)
{
  return 2 * i + 2;
}

// swaps in place one byte at a time, so it needs no scratch memory and can't
// fail
static void
swap_elements (priority_queue_t *queue, size_t i, size_t j)
{
  unsigned char *a = (unsigned char *)queue->data + (i * queue->elem_size);
  unsigned char *b = (unsigned char *)queue->data + (j * queue->elem_size);
  for (size_t k = 0; k < queue->elem_size; k++)
    {
      unsigned char tmp = a[k];
      a[k] = b[k];
      b[k] = tmp;
    }
}

static void
heapify_up (priority_queue_t *queue, size_t index)
{
  while (index > 0)
    {
      size_t parent_idx = parent (index);
      char *data = (char *)queue->data;

      if (queue->compare (data + (index * queue->elem_size),
                          data + (parent_idx * queue->elem_size))
          > 0)
        {
          swap_elements (queue, index, parent_idx);
          index = parent_idx;
        }
      else
        {
          break;
        }
    }
}

static void
heapify_down (priority_queue_t *queue, size_t index)
{
  char *data = (char *)queue->data;
  for (;;)
    {
      size_t max_index = index;
      size_t left = left_child (index);
      size_t right = right_child (index);

      if (left < queue->size
          && queue->compare (data + (left * queue->elem_size),
                             data + (max_index * queue->elem_size))
                 > 0)
        {
          max_index = left;
        }

      if (right < queue->size
          && queue->compare (data + (right * queue->elem_size),
                             data + (max_index * queue->elem_size))
                 > 0)
        {
          max_index = right;
        }

      if (max_index == index)
        {
          return;
        }

      swap_elements (queue, index, max_index);
      index = max_index;
    }
}

priority_queue_t *
priority_queue_create_with_allocator (size_t elem_size,
                                      size_t initial_capacity,
                                      int (*compare) (const void *a,
                                                      const void *b),
                                      cutils_allocator_t *allocator)
{
  g_last_error = PRIORITY_QUEUE_OK;

  if (elem_size == 0 || compare == NULL || allocator == NULL)
    {
      g_last_error = PRIORITY_QUEUE_INVALID_ARG;
      return NULL;
    }

  priority_queue_t *queue = cutils_allocate_aligned (
      allocator, sizeof (priority_queue_t), CUTILS_ALIGNMENT);
  if (queue == NULL)
    {
      g_last_error = PRIORITY_QUEUE_NO_MEMORY;
      return NULL;
    }

  queue->data = cutils_allocate_aligned (
      allocator, elem_size * initial_capacity, CUTILS_ALIGNMENT);
  if (queue->data == NULL)
    {
      cutils_deallocate (allocator, queue);
      g_last_error = PRIORITY_QUEUE_NO_MEMORY;
      return NULL;
    }

  queue->size = 0;
  queue->capacity = initial_capacity;
  queue->elem_size = elem_size;
  queue->allocator = allocator;
  queue->compare = compare;

  return queue;
}

priority_queue_t *
priority_queue_create (size_t elem_size, size_t initial_capacity,
                       int (*compare) (const void *a, const void *b))
{
  return priority_queue_create_with_allocator (
      elem_size, initial_capacity, compare, cutils_default_allocator ());
}

void
priority_queue_destroy (priority_queue_t *queue)
{
  g_last_error = PRIORITY_QUEUE_OK;

  if (queue == NULL)
    {
      g_last_error = PRIORITY_QUEUE_NULL_PTR;
      return;
    }

  cutils_deallocate (queue->allocator, queue->data);
  cutils_deallocate (queue->allocator, queue);
}

static bool
resize_if_needed (priority_queue_t *queue, uint32_t timeout_ms)
{
  if (queue->size < queue->capacity)
    {
      return true;
    }

  uint64_t start_time = cutils_get_current_time_ms ();
  size_t new_capacity = queue->capacity * 2;

  if (!priority_queue_can_perform_operation (queue,
                                             new_capacity * queue->elem_size))
    {
      g_last_error = PRIORITY_QUEUE_NO_MEMORY;
      return false;
    }

  void *new_data = cutils_allocate_aligned (
      queue->allocator, new_capacity * queue->elem_size, CUTILS_ALIGNMENT);
  if (new_data == NULL)
    {
      g_last_error = PRIORITY_QUEUE_NO_MEMORY;
      return false;
    }

  if (cutils_timeout_expired (start_time, timeout_ms))
    {
      cutils_deallocate (queue->allocator, new_data);
      g_last_error = PRIORITY_QUEUE_TIMEOUT;
      return false;
    }

  memcpy (new_data, queue->data, queue->size * queue->elem_size);
  cutils_deallocate (queue->allocator, queue->data);

  queue->data = new_data;
  queue->capacity = new_capacity;

  return true;
}

bool
priority_queue_push_timeout (priority_queue_t *queue, const void *elem,
                             uint32_t timeout_ms)
{
  g_last_error = PRIORITY_QUEUE_OK;

  if (queue == NULL || elem == NULL)
    {
      g_last_error = PRIORITY_QUEUE_NULL_PTR;
      return false;
    }

  if (!resize_if_needed (queue, timeout_ms))
    {
      return false;
    }

  memcpy ((char *)queue->data + (queue->size * queue->elem_size), elem,
          queue->elem_size);
  heapify_up (queue, queue->size);
  queue->size++;

  return true;
}

bool
priority_queue_push (priority_queue_t *queue, const void *elem)
{
  return priority_queue_push_timeout (queue, elem,
                                      CUTILS_MAX_OPERATION_TIME_MS);
}

bool
priority_queue_pop (priority_queue_t *queue, void *out_elem)
{
  g_last_error = PRIORITY_QUEUE_OK;

  if (queue == NULL || out_elem == NULL)
    {
      g_last_error = PRIORITY_QUEUE_NULL_PTR;
      return false;
    }

  if (queue->size == 0)
    {
      g_last_error = PRIORITY_QUEUE_EMPTY;
      return false;
    }

  memcpy (out_elem, queue->data, queue->elem_size);

  if (queue->size > 1)
    {
      memcpy (queue->data,
              (char *)queue->data + ((queue->size - 1) * queue->elem_size),
              queue->elem_size);
    }

  queue->size--;

  if (queue->size > 0)
    {
      heapify_down (queue, 0);
    }

  return true;
}

bool
priority_queue_peek (const priority_queue_t *queue, void *out_elem)
{
  g_last_error = PRIORITY_QUEUE_OK;

  if (queue == NULL || out_elem == NULL)
    {
      g_last_error = PRIORITY_QUEUE_NULL_PTR;
      return false;
    }

  if (queue->size == 0)
    {
      g_last_error = PRIORITY_QUEUE_EMPTY;
      return false;
    }

  memcpy (out_elem, queue->data, queue->elem_size);

  return true;
}

size_t
priority_queue_size (const priority_queue_t *queue)
{
  if (queue == NULL)
    {
      g_last_error = PRIORITY_QUEUE_NULL_PTR;
      return 0;
    }
  return queue->size;
}

size_t
priority_queue_capacity (const priority_queue_t *queue)
{
  if (queue == NULL)
    {
      g_last_error = PRIORITY_QUEUE_NULL_PTR;
      return 0;
    }
  return queue->capacity;
}

bool
priority_queue_is_empty (const priority_queue_t *queue)
{
  g_last_error = PRIORITY_QUEUE_OK;

  if (queue == NULL)
    {
      g_last_error = PRIORITY_QUEUE_NULL_PTR;
      return true;
    }

  return queue->size == 0;
}

bool
priority_queue_is_full (const priority_queue_t *queue)
{
  g_last_error = PRIORITY_QUEUE_OK;

  if (queue == NULL)
    {
      g_last_error = PRIORITY_QUEUE_NULL_PTR;
      return true;
    }

  return queue->size == queue->capacity;
}

bool
priority_queue_clear (priority_queue_t *queue)
{
  g_last_error = PRIORITY_QUEUE_OK;

  if (queue == NULL)
    {
      g_last_error = PRIORITY_QUEUE_NULL_PTR;
      return false;
    }

  queue->size = 0;
  return true;
}

size_t
priority_queue_memory_usage (const priority_queue_t *queue)
{
  if (queue == NULL)
    {
      g_last_error = PRIORITY_QUEUE_NULL_PTR;
      return 0;
    }
  return sizeof (priority_queue_t) + (queue->capacity * queue->elem_size);
}

bool
priority_queue_can_perform_operation (const priority_queue_t *queue,
                                      size_t required_capacity)
{
  if (queue == NULL)
    {
      g_last_error = PRIORITY_QUEUE_NULL_PTR;
      return false;
    }

  return cutils_can_allocate (queue->allocator, required_capacity,
                              CUTILS_ALIGNMENT);
}

priority_queue_result_t
priority_queue_get_error (void)
{
  return g_last_error;
}