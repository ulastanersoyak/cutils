#ifndef CUTILS_PRIORITY_QUEUE_H
#define CUTILS_PRIORITY_QUEUE_H

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
  size_t elem_size;
  cutils_allocator_t *allocator;
  int (*compare) (const void *a, const void *b);
} priority_queue_t;

typedef enum
{
  PRIORITY_QUEUE_OK = 0,
  PRIORITY_QUEUE_NULL_PTR = 1,
  PRIORITY_QUEUE_NO_MEMORY = 2,
  PRIORITY_QUEUE_INVALID_ARG = 3,
  PRIORITY_QUEUE_EMPTY = 4,
  PRIORITY_QUEUE_FULL = 5,
  PRIORITY_QUEUE_TIMEOUT = 6,
  PRIORITY_QUEUE_OVERFLOW = 7
} priority_queue_result_t;

/**
 * Creates a new priority queue with the specified allocator.
 *
 * @param elem_size Size of each element in bytes
 * @param initial_capacity Initial capacity of the queue
 * @param compare Comparison function for elements
 * @param allocator Allocator to use
 * @return Newly allocated priority queue or NULL on error
 */
priority_queue_t *priority_queue_create_with_allocator (
    size_t elem_size, size_t initial_capacity,
    int (*compare) (const void *a, const void *b),
    cutils_allocator_t *allocator);

/**
 * Creates a new priority queue using the default allocator.
 *
 * @param elem_size Size of each element in bytes
 * @param initial_capacity Initial capacity of the queue
 * @param compare Comparison function for elements
 * @return Newly allocated priority queue or NULL on error
 */
priority_queue_t *
priority_queue_create (size_t elem_size, size_t initial_capacity,
                       int (*compare) (const void *a, const void *b));

/**
 * Destroys priority queue and frees all allocated memory.
 *
 * @param queue Priority queue to destroy
 */
void priority_queue_destroy (priority_queue_t *queue);

/**
 * Pushes an element into the priority queue with timeout.
 *
 * @param queue Priority queue to push into
 * @param elem Element to push
 * @param timeout_ms Timeout in milliseconds
 * @return true if successful, false otherwise
 */
bool priority_queue_push_timeout (priority_queue_t *queue, const void *elem,
                                  uint32_t timeout_ms);

/**
 * Pushes an element into the priority queue.
 *
 * @param queue Priority queue to push into
 * @param elem Element to push
 * @return true if successful, false otherwise
 */
bool priority_queue_push (priority_queue_t *queue, const void *elem);

/**
 * Pops the highest priority element from the queue.
 *
 * @param queue Priority queue to pop from
 * @param out_elem Output parameter to store the popped element
 * @return true if successful, false otherwise
 */
bool priority_queue_pop (priority_queue_t *queue, void *out_elem);

/**
 * Gets the highest priority element without removing it.
 *
 * @param queue Priority queue to peek
 * @param out_elem Output parameter to store the highest priority element
 * @return true if successful, false otherwise
 */
bool priority_queue_peek (const priority_queue_t *queue, void *out_elem);

/**
 * Gets the number of elements in the priority queue.
 *
 * @param queue Priority queue to get size from
 * @return Number of elements
 */
size_t priority_queue_size (const priority_queue_t *queue);

/**
 * Gets the capacity of the priority queue.
 *
 * @param queue Priority queue to get capacity from
 * @return Capacity of the queue
 */
size_t priority_queue_capacity (const priority_queue_t *queue);

/**
 * Checks if the priority queue is empty.
 *
 * @param queue Priority queue to check
 * @return true if empty, false otherwise
 */
bool priority_queue_is_empty (const priority_queue_t *queue);

/**
 * Checks if the priority queue is full.
 *
 * @param queue Priority queue to check
 * @return true if full, false otherwise
 */
bool priority_queue_is_full (const priority_queue_t *queue);

/**
 * Clears all elements from the priority queue.
 *
 * @param queue Priority queue to clear
 * @return true if successful, false otherwise
 */
bool priority_queue_clear (priority_queue_t *queue);

/**
 * Gets the memory usage of the priority queue.
 *
 * @param queue Priority queue to get memory usage from
 * @return Memory usage in bytes
 */
size_t priority_queue_memory_usage (const priority_queue_t *queue);

/**
 * Checks if an operation would succeed without actually performing it.
 *
 * @param queue Priority queue to check
 * @param required_capacity Required capacity
 * @return true if operation would succeed, false otherwise
 */
bool priority_queue_can_perform_operation (const priority_queue_t *queue,
                                           size_t required_capacity);

/**
 * Gets the last priority queue operation error.
 *
 * @return Last error code
 */
priority_queue_result_t priority_queue_get_error (void);

#endif // CUTILS_PRIORITY_QUEUE_H