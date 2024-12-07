#ifndef CUTILS_EXPECTED_H
#define CUTILS_EXPECTED_H

#include <stdbool.h>
#include <stddef.h>

typedef enum
{
  EXPECTED_OK,
  EXPECTED_NULL_PTR,
  EXPECTED_NO_MEMORY,
  EXPECTED_INVALID_VALUE,
  EXPECTED_INVALID_ARG
} expected_result_t;

typedef struct
{
  union
  {
    void *value;
    void *error;
  } data;
  bool has_value; // true = value, false = error
} expected_t;

/**
 * Gets the last expected operation error.
 *
 * @return Last error code
 */
[[nodiscard]] expected_result_t expected_get_error (void);

/**
 * Creates a new expected with a success value.
 *
 * @param value Pointer to the value to copy
 * @param size Size of the value in bytes
 * @return Newly allocated expected or NULL on error
 * @note Sets error to EXPECTED_NULL_PTR if value is NULL
 * @note Sets error to EXPECTED_NO_MEMORY if memory allocation fails
 */
[[nodiscard]] expected_t *expected_success (const void *value, size_t size);

/**
 * Creates a new expected with an error value.
 *
 * @param error Pointer to the error value to copy
 * @param size Size of the error value in bytes
 * @return Newly allocated expected or NULL on error
 * @note Sets error to EXPECTED_NULL_PTR if error is NULL
 * @note Sets error to EXPECTED_NO_MEMORY if memory allocation fails
 */
[[nodiscard]] expected_t *expected_error (const void *error, size_t size);

/**
 * Checks if expected contains a success value.
 *
 * @param exp Expected to check
 * @return true if contains success value, false if contains error
 * @note Sets error to EXPECTED_NULL_PTR if exp is NULL
 */
bool expected_has_value (const expected_t *exp);

/**
 * Gets the success value if present.
 *
 * @param exp Expected to get value from
 * @return Pointer to success value or NULL if contains error
 * @note Sets error to EXPECTED_NULL_PTR if exp is NULL
 * @note Sets error to EXPECTED_INVALID_VALUE if contains error instead of
 * value
 */
[[nodiscard]] void *expected_value (const expected_t *exp);

/**
 * Gets the error value if present.
 *
 * @param exp Expected to get error from
 * @return Pointer to error value or NULL if contains success
 * @note Sets error to EXPECTED_NULL_PTR if exp is NULL
 * @note Sets error to EXPECTED_INVALID_VALUE if contains value instead of
 * error
 */
[[nodiscard]] void *expected_error_value (const expected_t *exp);

/**
 * Frees all memory associated with an expected.
 *
 * @param exp Expected to destroy
 * @note Does not free the contained value or error pointers
 */
void expected_destroy (expected_t *exp);

#endif // CUTILS_EXPECTED_H
