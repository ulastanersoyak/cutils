#ifndef CUTILS_EXPECTED_H
#define CUTILS_EXPECTED_H

#include "cutils/allocator.h"
#include "cutils/config.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum
{
  EXPECTED_OK = 0,
  EXPECTED_NULL_PTR = 1,
  EXPECTED_NO_MEMORY = 2,
  EXPECTED_INVALID_ARG = 3,
  EXPECTED_TIMEOUT = 4,
  EXPECTED_ERROR = 5
} expected_result_t;

typedef struct
{
  void *data;
  size_t size;
  expected_result_t error;
  cutils_allocator_t *allocator;
} expected_t;

/**
 * Creates a new expected value with the specified allocator.
 *
 * @param size Size of the value in bytes
 * @param allocator Allocator to use
 * @return Newly allocated expected value or NULL on error
 */
expected_t *expected_create_with_allocator (size_t size,
                                            cutils_allocator_t *allocator);

/**
 * Creates a new expected value using the default allocator.
 *
 * @param size Size of the value in bytes
 * @return Newly allocated expected value or NULL on error
 */
expected_t *expected_create (size_t size);

/**
 * Creates a new expected value from data with the specified allocator.
 *
 * @param data Data to copy
 * @param size Size of the data in bytes
 * @param allocator Allocator to use
 * @return Newly allocated expected value or NULL on error
 */
expected_t *expected_from_data_with_allocator (const void *data, size_t size,
                                               cutils_allocator_t *allocator);

/**
 * Creates a new expected value from data using the default allocator.
 *
 * @param data Data to copy
 * @param size Size of the data in bytes
 * @return Newly allocated expected value or NULL on error
 */
expected_t *expected_from_data (const void *data, size_t size);

/**
 * Creates a new expected error with the specified allocator.
 *
 * @param error Error code
 * @param allocator Allocator to use
 * @return Newly allocated expected error or NULL on error
 */
expected_t *expected_from_error_with_allocator (expected_result_t error,
                                                cutils_allocator_t *allocator);

/**
 * Creates a new expected error using the default allocator.
 *
 * @param error Error code
 * @return Newly allocated expected error or NULL on error
 */
expected_t *expected_from_error (expected_result_t error);

/**
 * Destroys expected value and frees all allocated memory.
 *
 * @param expected Expected value to destroy
 */
void expected_destroy (expected_t *expected);

/**
 * Gets the data from the expected value.
 *
 * @param expected Expected value to get data from
 * @return Pointer to data or NULL if error
 */
void *expected_get_data (const expected_t *expected);

/**
 * Gets the error from the expected value.
 *
 * @param expected Expected value to get error from
 * @return Error code
 */
expected_result_t expected_get_error (const expected_t *expected);

/**
 * Checks if the expected value has a value.
 *
 * @param expected Expected value to check
 * @return true if has value, false otherwise
 */
bool expected_has_value (const expected_t *expected);

/**
 * Checks if the expected value has an error.
 *
 * @param expected Expected value to check
 * @return true if has error, false otherwise
 */
bool expected_has_error (const expected_t *expected);

/**
 * Gets the size of the expected value.
 *
 * @param expected Expected value to get size from
 * @return Size in bytes
 */
size_t expected_size (const expected_t *expected);

/**
 * Gets the memory usage of the expected value.
 *
 * @param expected Expected value to get memory usage from
 * @return Memory usage in bytes
 */
size_t expected_memory_usage (const expected_t *expected);

/**
 * Gets the last expected operation error.
 *
 * @return Last error code
 */
expected_result_t expected_get_last_error (void);

#endif // CUTILS_EXPECTED_H
