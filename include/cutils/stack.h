#ifndef CUTILS_STACK_H
#define CUTILS_STACK_H

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
} stack_t;

typedef enum
{
  STACK_OK = 0,
  STACK_NULL_PTR = 1,
  STACK_NO_MEMORY = 2,
  STACK_INVALID_ARG = 3,
  STACK_EMPTY = 4,
  STACK_FULL = 5,
  STACK_TIMEOUT = 6,
  STACK_OVERFLOW = 7
} stack_result_t;

/**
 * Creates a new stack with the specified allocator.
 *
 * @param elem_size Size of each element in bytes
 * @param initial_capacity Initial capacity of the stack
 * @param allocator Allocator to use
 * @return Newly allocated stack or NULL on error
 */
stack_t *stack_create_with_allocator (size_t elem_size,
                                      size_t initial_capacity,
                                      cutils_allocator_t *allocator);

/**
 * Creates a new stack using the default allocator.
 *
 * @param elem_size Size of each element in bytes
 * @param initial_capacity Initial capacity of the stack
 * @return Newly allocated stack or NULL on error
 */
stack_t *stack_create (size_t elem_size, size_t initial_capacity);

/**
 * Destroys stack and frees all allocated memory.
 *
 * @param stack Stack to destroy
 */
void stack_destroy (stack_t *stack);

/**
 * Pushes an element onto the stack with timeout.
 *
 * @param stack Stack to push onto
 * @param elem Element to push
 * @param timeout_ms Timeout in milliseconds
 * @return true if successful, false otherwise
 */
bool stack_push_timeout (stack_t *stack, const void *elem,
                         uint32_t timeout_ms);

/**
 * Pushes an element onto the stack.
 *
 * @param stack Stack to push onto
 * @param elem Element to push
 * @return true if successful, false otherwise
 */
bool stack_push (stack_t *stack, const void *elem);

/**
 * Pops an element from the stack.
 *
 * @param stack Stack to pop from
 * @param out_elem Output parameter to store the popped element
 * @return true if successful, false otherwise
 */
bool stack_pop (stack_t *stack, void *out_elem);

/**
 * Gets the top element of the stack without removing it.
 *
 * @param stack Stack to peek
 * @param out_elem Output parameter to store the top element
 * @return true if successful, false otherwise
 */
bool stack_peek (const stack_t *stack, void *out_elem);

/**
 * Gets the number of elements in the stack.
 *
 * @param stack Stack to get size from
 * @return Number of elements
 */
size_t stack_size (const stack_t *stack);

/**
 * Gets the capacity of the stack.
 *
 * @param stack Stack to get capacity from
 * @return Capacity of the stack
 */
size_t stack_capacity (const stack_t *stack);

/**
 * Checks if the stack is empty.
 *
 * @param stack Stack to check
 * @return true if empty, false otherwise
 */
bool stack_is_empty (const stack_t *stack);

/**
 * Checks if the stack is full.
 *
 * @param stack Stack to check
 * @return true if full, false otherwise
 */
bool stack_is_full (const stack_t *stack);

/**
 * Clears all elements from the stack.
 *
 * @param stack Stack to clear
 * @return true if successful, false otherwise
 */
bool stack_clear (stack_t *stack);

/**
 * Gets the memory usage of the stack.
 *
 * @param stack Stack to get memory usage from
 * @return Memory usage in bytes
 */
size_t stack_memory_usage (const stack_t *stack);

/**
 * Checks if an operation would succeed without actually performing it.
 *
 * @param stack Stack to check
 * @param required_capacity Required capacity
 * @return true if operation would succeed, false otherwise
 */
bool stack_can_perform_operation (const stack_t *stack,
                                  size_t required_capacity);

/**
 * Gets the last stack operation error.
 *
 * @return Last error code
 */
stack_result_t stack_get_error (void);

#endif // CUTILS_STACK_H
