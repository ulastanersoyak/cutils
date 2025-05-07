#ifndef CUTILS_VECTOR_H
#define CUTILS_VECTOR_H

#include "cutils/allocator.h"
#include "cutils/config.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct
{
  void *data;
  size_t len;
  size_t capacity;
  size_t elem_len;
  cutils_allocator_t *allocator;
} vector_t;

typedef enum
{
  VECTOR_OK = 0,
  VECTOR_NULL_PTR = 1,
  VECTOR_NO_MEMORY = 2,
  VECTOR_INVALID_ARG = 3,
  VECTOR_OUT_OF_RANGE = 4,
  VECTOR_OVERFLOW = 5,
  VECTOR_TIMEOUT = 6,
  VECTOR_PRIORITY_ERROR = 7
} vector_result_t;

/**
 * Gets the last vector operation error.
 *
 * @return Last error code
 */
[[nodiscard]] vector_result_t vector_get_error (void);

/**
 * Creates a new stack with the specified allocator.
 *
 * @param elem_size Size of each element in bytes
 * @param initial_capacity Initial capacity of the vector
 * @param allocator Allocator to use
 * @return Newly allocated stack or NULL on error
 */
vector_t *vector_create_with_allocator (size_t init_capacity, size_t elem_len,
                                        cutils_allocator_t *allocator);

/**
 * Creates a new vector with specified element size and initial capacity.
 *
 * @param init_capacity Initial capacity (0 for default)
 * @param elem_len Size of each element in bytes
 * @return Newly allocated vector or NULL on error
 * @note Sets error to VECTOR_INVALID_ARG if elem_len is 0
 * @note Sets error to VECTOR_OVERFLOW if capacity too large
 * @note Sets error to VECTOR_NO_MEMORY if allocation fails
 */
[[nodiscard]] vector_t *vector_create (size_t init_capacity, size_t elem_len);

/**
 * Creates a deep copy of a vector.
 *
 * @param vec Vector to copy
 * @return Newly allocated copy or NULL on error
 * @note Sets error to VECTOR_NULL_PTR if vec is NULL
 * @note Sets error to VECTOR_NO_MEMORY if allocation fails
 */
[[nodiscard]] vector_t *vector_copy (const vector_t *vec);

/**
 * Frees all memory associated with vector.
 *
 * @param vec Vector to destroy
 * @note Sets error to VECTOR_NULL_PTR if vec is NULL
 */
void vector_destroy (vector_t *vec);

/**
 * Adds an element to end of vector.
 *
 * @param vec Vector to append to
 * @param elem Element to append
 * @return true if successful, false otherwise
 * @note Sets error to VECTOR_NULL_PTR if any parameter is NULL
 * @note Sets error to VECTOR_NO_MEMORY if reallocation fails
 */
bool vector_push (vector_t *vec, const void *elem);

/**
 * Removes and returns last element.
 *
 * @param vec Vector to pop from
 * @param out Buffer to store popped element
 * @return true if successful, false otherwise
 * @note Sets error to VECTOR_NULL_PTR if any parameter is NULL
 * @note Sets error to VECTOR_OUT_OF_RANGE if vector is empty
 */
bool vector_pop (vector_t *vec, void *out);

/**
 * Gets element at specified index.
 *
 * @param vec Vector to get from
 * @param index Index of element
 * @param out Buffer to store element
 * @return true if successful, false otherwise
 * @note Sets error to VECTOR_NULL_PTR if any parameter is NULL
 * @note Sets error to VECTOR_OUT_OF_RANGE if index invalid
 */
bool vector_get (const vector_t *vec, size_t index, void *out);

/**
 * Sets element at specified index.
 *
 * @param vec Vector to modify
 * @param index Index to set
 * @param elem New element value
 * @return true if successful, false otherwise
 * @note Sets error to VECTOR_NULL_PTR if any parameter is NULL
 * @note Sets error to VECTOR_OUT_OF_RANGE if index invalid
 */
bool vector_set (vector_t *vec, size_t index, const void *elem);

/**
 * Inserts element at specified index.
 *
 * @param vec Vector to insert into
 * @param index Index to insert at
 * @param elem Element to insert
 * @return true if successful, false otherwise
 * @note Sets error to VECTOR_NULL_PTR if any parameter is NULL
 * @note Sets error to VECTOR_OUT_OF_RANGE if index invalid
 * @note Sets error to VECTOR_NO_MEMORY if reallocation fails
 */
bool vector_insert (vector_t *vec, size_t index, const void *elem);

/**
 * Removes element at specified index.
 *
 * @param vec Vector to remove from
 * @param index Index to remove
 * @param out Optional buffer to store removed element
 * @return true if successful, false otherwise
 * @note Sets error to VECTOR_NULL_PTR if vec is NULL
 * @note Sets error to VECTOR_OUT_OF_RANGE if index invalid
 */
bool vector_remove (vector_t *vec, size_t index, void *out);

/**
 * Ensures vector capacity is at least specified size.
 *
 * @param vec Vector to reserve capacity for
 * @param capacity Minimum capacity to ensure
 * @return true if successful, false otherwise
 * @note Sets error to VECTOR_NULL_PTR if vec is NULL
 * @note Sets error to VECTOR_OVERFLOW if capacity too large
 * @note Sets error to VECTOR_NO_MEMORY if reallocation fails
 */
bool vector_reserve (vector_t *vec, size_t capacity);

/**
 * Reduces capacity to length.
 *
 * @param vec Vector to shrink
 * @return true if successful, false otherwise
 * @note Sets error to VECTOR_NULL_PTR if vec is NULL
 * @note Sets error to VECTOR_NO_MEMORY if reallocation fails
 */
bool vector_shrink (vector_t *vec);

/**
 * Removes all elements from vector.
 *
 * @param vec Vector to clear
 * @return true if successful, false otherwise
 * @note Sets error to VECTOR_NULL_PTR if vec is NULL
 */
bool vector_clear (vector_t *vec);

/**
 * Checks if vector is empty.
 *
 * @param vec Vector to check
 * @return true if empty, false otherwise
 * @note Sets error to VECTOR_NULL_PTR if vec is NULL
 */
bool vector_is_empty (const vector_t *vec);

/**
 * Gets the last element without removing it.
 *
 * @param vec Vector to peek from
 * @param out Buffer to store last element
 * @return true if successful, false otherwise
 * @note Sets error to VECTOR_NULL_PTR if any parameter is NULL
 * @note Sets error to VECTOR_OUT_OF_RANGE if vector is empty
 */
bool vector_back (const vector_t *vec, void *out);

/**
 * Gets the current length of the vector.
 *
 * @param vec Vector to get length from
 * @return Current length of the vector
 * @note Sets error to VECTOR_NULL_PTR if vec is NULL
 */
size_t vector_length (const vector_t *vec);

/**
 * Gets the current capacity of the vector.
 *
 * @param vec Vector to get capacity from
 * @return Current capacity of the vector
 * @note Sets error to VECTOR_NULL_PTR if vec is NULL
 */
size_t vector_capacity (const vector_t *vec);

/**
 * Gets the element size of the vector.
 *
 * @param vec Vector to get element size from
 * @return Size of each element in the vector
 * @note Sets error to VECTOR_NULL_PTR if vec is NULL
 */
size_t vector_element_size (const vector_t *vec);

/**
 * Gets memory usage statistics of the vector.
 *
 * @param vec Vector to get memory usage from
 * @return Memory usage of the vector in bytes
 * @note Sets error to VECTOR_NULL_PTR if vec is NULL
 */
size_t vector_memory_usage (const vector_t *vec);

/**
 * Checks if an operation would succeed without actually performing it.
 *
 * @param vec Vector to check
 * @param required_capacity Minimum capacity required
 * @return true if operation would succeed, false otherwise
 * @note Sets error to VECTOR_NULL_PTR if vec is NULL
 */
bool vector_can_perform_operation (const vector_t *vec,
                                   size_t required_capacity);

#endif // CUTILS_VECTOR_H
