#ifndef CUTILS_VECTOR_H
#define CUTILS_VECTOR_H

#include <stdbool.h>
#include <stddef.h>

typedef enum
{
  VECTOR_OK,
  VECTOR_NULL_PTR,
  VECTOR_INVALID_ARG,
  VECTOR_NO_MEMORY,
  VECTOR_OVERFLOW,
  VECTOR_OUT_OF_RANGE
} vector_result_t;

#define VECTOR_INIT_CAPACITY 8
#define VECTOR_GROWTH_FACTOR 2 // double the capacity when growing

typedef struct
{
  void *data;
  size_t len;
  size_t capacity;
  size_t elem_len;
} vector_t;

/**
 * Gets the last vector operation error.
 *
 * @return Last error code
 */
[[nodiscard]] vector_result_t vector_get_error (void);

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

#endif // CUTILS_VECTOR_H
