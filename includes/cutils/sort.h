#ifndef CUTILS_SORT_H
#define CUTILS_SORT_H

#include <stdbool.h>
#include <stddef.h>

typedef enum
{
  SORT_OK,
  SORT_NULL_PTR,
  SORT_INVALID_ARG,
  SORT_NO_MEMORY
} sort_result_t;

typedef enum
{
  SORT_ALGORITHM_QUICK,
  SORT_ALGORITHM_MERGE,
  SORT_ALGORITHM_HEAP,
  SORT_ALGORITHM_INSERTION,
  SORT_ALGORITHM_SHELL
} sort_algorithm_t;

// comparison function type definition
// should return: negative if a < b, 0 if a == b, positive if a > b
typedef int (*sort_compare_fn) (const void *a, const void *b);

/**
 * Gets the last sort operation error.
 *
 * @return Last error code
 */
[[nodiscard]] sort_result_t sort_get_error (void);

/**
 * Sorts array using specified algorithm.
 *
 * @param array Pointer to array to sort
 * @param length Number of elements in array
 * @param elem_size Size of each element in bytes
 * @param compare Comparison function
 * @param algorithm Sorting algorithm to use
 * @return true if sort successful, false on error
 * @note Sets error to SORT_NULL_PTR if array or compare is NULL
 * @note Sets error to SORT_INVALID_ARG if length is 0 or elem_size is 0
 * @note Sets error to SORT_NO_MEMORY if temporary allocation fails
 */
[[nodiscard]] bool sort_array (void *array, size_t length, size_t elem_size,
                               sort_compare_fn compare,
                               sort_algorithm_t algorithm);

/**
 * Sorts array using quicksort algorithm.
 *
 * @param array Pointer to array to sort
 * @param length Number of elements in array
 * @param elem_size Size of each element in bytes
 * @param compare Comparison function
 * @return true if sort successful, false on error
 * @note Sets error to SORT_NULL_PTR if array or compare is NULL
 * @note Sets error to SORT_INVALID_ARG if length is 0 or elem_size is 0
 * @note Sets error to SORT_NO_MEMORY if temporary allocation fails
 */
[[nodiscard]] bool sort_quick (void *array, size_t length, size_t elem_size,
                               sort_compare_fn compare);

/**
 * Sorts array using mergesort algorithm.
 *
 * @param array Pointer to array to sort
 * @param length Number of elements in array
 * @param elem_size Size of each element in bytes
 * @param compare Comparison function
 * @return true if sort successful, false on error
 * @note Sets error to SORT_NULL_PTR if array or compare is NULL
 * @note Sets error to SORT_INVALID_ARG if length is 0 or elem_size is 0
 * @note Sets error to SORT_NO_MEMORY if temporary allocation fails
 */
[[nodiscard]] bool sort_merge (void *array, size_t length, size_t elem_size,
                               sort_compare_fn compare);

/**
 * Sorts array using heapsort algorithm.
 *
 * @param array Pointer to array to sort
 * @param length Number of elements in array
 * @param elem_size Size of each element in bytes
 * @param compare Comparison function
 * @return true if sort successful, false on error
 * @note Sets error to SORT_NULL_PTR if array or compare is NULL
 * @note Sets error to SORT_INVALID_ARG if length is 0 or elem_size is 0
 * @note Sets error to SORT_NO_MEMORY if temporary allocation fails
 */
[[nodiscard]] bool sort_heap (void *array, size_t length, size_t elem_size,
                              sort_compare_fn compare);

/**
 * Sorts array using insertion sort algorithm.
 *
 * @param array Pointer to array to sort
 * @param length Number of elements in array
 * @param elem_size Size of each element in bytes
 * @param compare Comparison function
 * @return true if sort successful, false on error
 * @note Sets error to SORT_NULL_PTR if array or compare is NULL
 * @note Sets error to SORT_INVALID_ARG if length is 0 or elem_size is 0
 * @note Sets error to SORT_NO_MEMORY if temporary allocation fails
 */
[[nodiscard]] bool sort_insertion (void *array, size_t length,
                                   size_t elem_size, sort_compare_fn compare);

/**
 * Sorts array using shell sort algorithm.
 *
 * @param array Pointer to array to sort
 * @param length Number of elements in array
 * @param elem_size Size of each element in bytes
 * @param compare Comparison function
 * @return true if sort successful, false on error
 * @note Sets error to SORT_NULL_PTR if array or compare is NULL
 * @note Sets error to SORT_INVALID_ARG if length is 0 or elem_size is 0
 * @note Sets error to SORT_NO_MEMORY if temporary allocation fails
 */
[[nodiscard]] bool sort_shell (void *array, size_t length, size_t elem_size,
                               sort_compare_fn compare);

/**
 * Checks if array is sorted in ascending order.
 *
 * @param array Pointer to array to check
 * @param length Number of elements in array
 * @param elem_size Size of each element in bytes
 * @param compare Comparison function
 * @return true if array is sorted, false if not sorted or error
 * @note Sets error to SORT_NULL_PTR if array or compare is NULL
 * @note Sets error to SORT_INVALID_ARG if length is 0 or elem_size is 0
 */
[[nodiscard]] bool sort_is_sorted (const void *array, size_t length,
                                   size_t elem_size, sort_compare_fn compare);

// common comparison functions
[[nodiscard]] int sort_compare_int (const void *a, const void *b);
[[nodiscard]] int sort_compare_uint (const void *a, const void *b);
[[nodiscard]] int sort_compare_float (const void *a, const void *b);
[[nodiscard]] int sort_compare_double (const void *a, const void *b);
[[nodiscard]] int sort_compare_string (const void *a, const void *b);

#endif // CUTILS_SORT_H
