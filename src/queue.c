#include "cutils/queue.h"
#include "capacity.h"
#include "cutils/config.h"
#include "cutils/time.h"
#include <stdint.h>
#include <string.h>
#include <threads.h>

static thread_local queue_result_t g_last_error = QUEUE_OK;

queue_t *
queue_create_with_allocator (size_t elem_size, size_t initial_capacity,
                             cutils_allocator_t *allocator)
{
  g_last_error = QUEUE_OK;

  if (elem_size == 0 || allocator == NULL)
    {
      g_last_error = QUEUE_INVALID_ARG;
      return NULL;
    }

  size_t bytes;
  if (!cutils_array_bytes (initial_capacity, elem_size, &bytes))
    {
      g_last_error = QUEUE_OVERFLOW;
      return NULL;
    }

  queue_t *queue = cutils_allocate_aligned (allocator, sizeof (queue_t),
                                            CUTILS_ALIGNMENT);
  if (queue == NULL)
    {
      g_last_error = QUEUE_NO_MEMORY;
      return NULL;
    }

  // with capacity 0 the buffer is allocated on the first enqueue
  queue->data = NULL;
  if (bytes > 0)
    {
      queue->data
          = cutils_allocate_aligned (allocator, bytes, CUTILS_ALIGNMENT);
      if (queue->data == NULL)
        {
          cutils_deallocate (allocator, queue);
          g_last_error = QUEUE_NO_MEMORY;
          return NULL;
        }
    }

  queue->size = 0;
  queue->capacity = initial_capacity;
  queue->head = 0;
  queue->tail = 0;
  queue->elem_size = elem_size;
  queue->allocator = allocator;

  return queue;
}

queue_t *
queue_create (size_t elem_size, size_t initial_capacity)
{
  return queue_create_with_allocator (elem_size, initial_capacity,
                                      cutils_default_allocator ());
}

void
queue_destroy (queue_t *queue)
{
  g_last_error = QUEUE_OK;

  if (queue == NULL)
    {
      g_last_error = QUEUE_NULL_PTR;
      return;
    }

  cutils_deallocate (queue->allocator, queue->data);
  cutils_deallocate (queue->allocator, queue);
}

static bool
resize_if_needed (queue_t *queue, uint32_t timeout_ms)
{
  if (queue->size < queue->capacity)
    {
      return true;
    }

  uint64_t start_time = cutils_get_current_time_ms ();

  size_t new_capacity;
  size_t bytes;
  if (!cutils_next_capacity (queue->capacity, queue->size + 1, SIZE_MAX,
                             &new_capacity)
      || !cutils_array_bytes (new_capacity, queue->elem_size, &bytes))
    {
      g_last_error = QUEUE_OVERFLOW;
      return false;
    }

  char *new_data
      = cutils_allocate_aligned (queue->allocator, bytes, CUTILS_ALIGNMENT);
  if (new_data == NULL)
    {
      g_last_error = QUEUE_NO_MEMORY;
      return false;
    }

  // unwrap the ring: the size elements starting at head may run past the
  // end of the buffer and continue from index 0
  if (queue->size > 0)
    {
      size_t first_part = queue->capacity - queue->head;
      if (first_part > queue->size)
        {
          first_part = queue->size;
        }
      memcpy (new_data, (char *)queue->data + (queue->head * queue->elem_size),
              first_part * queue->elem_size);
      memcpy (new_data + (first_part * queue->elem_size), queue->data,
              (queue->size - first_part) * queue->elem_size);
    }

  cutils_deallocate (queue->allocator, queue->data);
  queue->data = new_data;
  queue->capacity = new_capacity;
  queue->head = 0;
  queue->tail = queue->size;

  if (cutils_timeout_expired (start_time, timeout_ms))
    {
      g_last_error = QUEUE_TIMEOUT;
      return false;
    }

  return true;
}

bool
queue_enqueue_timeout (queue_t *queue, const void *elem, uint32_t timeout_ms)
{
  g_last_error = QUEUE_OK;

  if (queue == NULL || elem == NULL)
    {
      g_last_error = QUEUE_NULL_PTR;
      return false;
    }

  if (!resize_if_needed (queue, timeout_ms))
    {
      return false;
    }

  memcpy ((char *)queue->data + (queue->tail * queue->elem_size), elem,
          queue->elem_size);
  queue->tail = (queue->tail + 1) % queue->capacity;
  queue->size++;

  return true;
}

bool
queue_enqueue (queue_t *queue, const void *elem)
{
  return queue_enqueue_timeout (queue, elem, CUTILS_MAX_OPERATION_TIME_MS);
}

bool
queue_dequeue (queue_t *queue, void *out_elem)
{
  g_last_error = QUEUE_OK;

  if (queue == NULL || out_elem == NULL)
    {
      g_last_error = QUEUE_NULL_PTR;
      return false;
    }

  if (queue->size == 0)
    {
      g_last_error = QUEUE_EMPTY;
      return false;
    }

  memcpy (out_elem, (char *)queue->data + (queue->head * queue->elem_size),
          queue->elem_size);
  queue->head = (queue->head + 1) % queue->capacity;
  queue->size--;

  return true;
}

bool
queue_peek (const queue_t *queue, void *out_elem)
{
  g_last_error = QUEUE_OK;

  if (queue == NULL || out_elem == NULL)
    {
      g_last_error = QUEUE_NULL_PTR;
      return false;
    }

  if (queue->size == 0)
    {
      g_last_error = QUEUE_EMPTY;
      return false;
    }

  memcpy (out_elem, (char *)queue->data + (queue->head * queue->elem_size),
          queue->elem_size);

  return true;
}

size_t
queue_size (const queue_t *queue)
{
  if (queue == NULL)
    {
      g_last_error = QUEUE_NULL_PTR;
      return 0;
    }
  return queue->size;
}

size_t
queue_capacity (const queue_t *queue)
{
  if (queue == NULL)
    {
      g_last_error = QUEUE_NULL_PTR;
      return 0;
    }
  return queue->capacity;
}

bool
queue_is_empty (const queue_t *queue)
{
  g_last_error = QUEUE_OK;

  if (queue == NULL)
    {
      g_last_error = QUEUE_NULL_PTR;
      return true;
    }

  return queue->size == 0;
}

bool
queue_is_full (const queue_t *queue)
{
  g_last_error = QUEUE_OK;

  if (queue == NULL)
    {
      g_last_error = QUEUE_NULL_PTR;
      return true;
    }

  return queue->size == queue->capacity;
}

bool
queue_clear (queue_t *queue)
{
  g_last_error = QUEUE_OK;

  if (queue == NULL)
    {
      g_last_error = QUEUE_NULL_PTR;
      return false;
    }

  queue->size = 0;
  queue->head = 0;
  queue->tail = 0;

  return true;
}

size_t
queue_memory_usage (const queue_t *queue)
{
  if (queue == NULL)
    {
      g_last_error = QUEUE_NULL_PTR;
      return 0;
    }
  return sizeof (queue_t) + (queue->capacity * queue->elem_size);
}

bool
queue_can_perform_operation (const queue_t *queue, size_t required_capacity)
{
  if (queue == NULL)
    {
      g_last_error = QUEUE_NULL_PTR;
      return false;
    }

  return cutils_can_allocate (queue->allocator, required_capacity,
                              CUTILS_ALIGNMENT);
}

queue_result_t
queue_get_error (void)
{
  return g_last_error;
}