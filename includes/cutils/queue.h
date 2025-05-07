#ifndef CUTILS_QUEUE_H
#define CUTILS_QUEUE_H

#include "cutils/allocator.h"
#include "cutils/config.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct
{
  void *data;
  size_t size;
  size_t capacity;
  size_t head;
  size_t tail;
  size_t elem_size;
  cutils_allocator_t *allocator;
} queue_t;

typedef enum
{
  QUEUE_OK = 0,
  QUEUE_NULL_PTR = 1,
  QUEUE_NO_MEMORY = 2,
  QUEUE_INVALID_ARG = 3,
  QUEUE_EMPTY = 4,
  QUEUE_FULL = 5,
  QUEUE_TIMEOUT = 6,
  QUEUE_OVERFLOW = 7
} queue_result_t;

/**
 * Creates a new queue with the specified allocator.
 *
 * @param elem_size Size of each element in bytes
 * @param initial_capacity Initial capacity of the queue
 * @param allocator Allocator to use
 * @return Newly allocated queue or NULL on error
 */
queue_t *queue_create_with_allocator (size_t elem_size,
                                      size_t initial_capacity,
                                      cutils_allocator_t *allocator);

/**
 * Creates a new queue using the default allocator.
 *
 * @param elem_size Size of each element in bytes
 * @param initial_capacity Initial capacity of the queue
 * @return Newly allocated queue or NULL on error
 */
queue_t *queue_create (size_t elem_size, size_t initial_capacity);

/**
 * Destroys queue and frees all allocated memory.
 *
 * @param queue Queue to destroy
 */
void queue_destroy (queue_t *queue);

/**
 * Enqueues an element with timeout.
 *
 * @param queue Queue to enqueue into
 * @param elem Element to enqueue
 * @param timeout_ms Timeout in milliseconds
 * @return true if successful, false otherwise
 */
bool queue_enqueue_timeout (queue_t *queue, const void *elem,
                            uint32_t timeout_ms);

/**
 * Enqueues an element.
 *
 * @param queue Queue to enqueue into
 * @param elem Element to enqueue
 * @return true if successful, false otherwise
 */
bool queue_enqueue (queue_t *queue, const void *elem);

/**
 * Dequeues an element.
 *
 * @param queue Queue to dequeue from
 * @param out_elem Output parameter to store the dequeued element
 * @return true if successful, false otherwise
 */
bool queue_dequeue (queue_t *queue, void *out_elem);

/**
 * Gets the front element without removing it.
 *
 * @param queue Queue to peek
 * @param out_elem Output parameter to store the front element
 * @return true if successful, false otherwise
 */
bool queue_peek (const queue_t *queue, void *out_elem);

/**
 * Gets the number of elements in the queue.
 *
 * @param queue Queue to get size from
 * @return Number of elements
 */
size_t queue_size (const queue_t *queue);

/**
 * Gets the capacity of the queue.
 *
 * @param queue Queue to get capacity from
 * @return Capacity of the queue
 */
size_t queue_capacity (const queue_t *queue);

/**
 * Checks if the queue is empty.
 *
 * @param queue Queue to check
 * @return true if empty, false otherwise
 */
bool queue_is_empty (const queue_t *queue);

/**
 * Checks if the queue is full.
 *
 * @param queue Queue to check
 * @return true if full, false otherwise
 */
bool queue_is_full (const queue_t *queue);

/**
 * Clears all elements from the queue.
 *
 * @param queue Queue to clear
 * @return true if successful, false otherwise
 */
bool queue_clear (queue_t *queue);

/**
 * Gets the memory usage of the queue.
 *
 * @param queue Queue to get memory usage from
 * @return Memory usage in bytes
 */
size_t queue_memory_usage (const queue_t *queue);

/**
 * Checks if an operation would succeed without actually performing it.
 *
 * @param queue Queue to check
 * @param required_capacity Required capacity
 * @return true if operation would succeed, false otherwise
 */
bool queue_can_perform_operation (const queue_t *queue,
                                  size_t required_capacity);

/**
 * Gets the last queue operation error.
 *
 * @return Last error code
 */
queue_result_t queue_get_error (void);

#endif // CUTILS_QUEUE_H