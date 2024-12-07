#ifndef CUTILS_STACK_H
#define CUTILS_STACK_H

#include <stdbool.h>
#include <stddef.h>

typedef enum
{
  STACK_OK,
  STACK_NULL_PTR,
  STACK_INVALID_ARG,
  STACK_OUT_OF_RANGE
} stack_result_t;

typedef struct
{
  void *data;
  size_t top;
  size_t capacity;
  size_t elem_len;
} stack_t;

#define STACK_DEFAULT_CAPACITY 1024

/**
 * Gets the last stack operation error.
 *
 * @return Last error code
 */
[[nodiscard]] stack_result_t stack_get_error (void);

/**
 * Creates a new stack for elements of given size.
 *
 * @param capacity Initial capacity (0 for default)
 * @param elem_len Size of each element in bytes
 * @return Initialized stack or NULL on error
 * @note Sets error to STACK_INVALID_ARG if elem_len is 0
 * @note Sets error to STACK_OUT_OF_RANGE if capacity too large
 */
[[nodiscard]] stack_t *stack_create (size_t capacity, size_t elem_len);

/**
 * Frees stack resources.
 *
 * @param stack Stack to destroy
 * @note Sets error to STACK_NULL_PTR if stack is NULL
 */
void stack_destroy (stack_t *stack);

/**
 * Pushes an element onto the stack.
 *
 * @param stack Stack to push onto
 * @param elem Element to push
 * @return true if successful, false otherwise
 * @note Sets error to STACK_NULL_PTR if any parameter is NULL
 * @note Sets error to STACK_OUT_OF_RANGE if stack is full
 */
bool stack_push (stack_t *stack, const void *elem);

/**
 * Pops top element from the stack.
 *
 * @param stack Stack to pop from
 * @param out Buffer to store popped element
 * @return true if successful, false otherwise
 * @note Sets error to STACK_NULL_PTR if any parameter is NULL
 * @note Sets error to STACK_OUT_OF_RANGE if stack is empty
 */
bool stack_pop (stack_t *stack, void *out);

/**
 * Gets top element without removing it.
 *
 * @param stack Stack to peek from
 * @param out Buffer to store top element
 * @return true if successful, false otherwise
 * @note Sets error to STACK_NULL_PTR if any parameter is NULL
 * @note Sets error to STACK_OUT_OF_RANGE if stack is empty
 */
bool stack_peek (const stack_t *stack, void *out);

/**
 * Removes all elements from stack.
 *
 * @param stack Stack to clear
 * @return true if successful, false otherwise
 * @note Sets error to STACK_NULL_PTR if stack is NULL
 */
bool stack_clear (stack_t *stack);

/**
 * Checks if stack is empty.
 *
 * @param stack Stack to check
 * @return true if empty, false otherwise
 * @note Sets error to STACK_NULL_PTR if stack is NULL
 */
bool stack_is_empty (const stack_t *stack);

#endif // CUTILS_STACK_H
