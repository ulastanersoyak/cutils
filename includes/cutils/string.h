#ifndef CUTILS_STRING_H
#define CUTILS_STRING_H

#include "cutils/allocator.h"
#include "cutils/config.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct
{
  char *data;
  size_t length;
  size_t capacity;
  cutils_allocator_t *allocator;
} string_t;

typedef enum
{
  STRING_OK = 0,
  STRING_NULL_PTR = 1,
  STRING_NO_MEMORY = 2,
  STRING_INVALID_ARG = 3,
  STRING_OVERFLOW = 4,
  STRING_TIMEOUT = 5,
  STRING_NOT_FOUND = 6
} string_result_t;

/**
 * Creates a new string with the specified allocator.
 *
 * @param initial_capacity Initial capacity of the string
 * @param allocator Allocator to use
 * @return Newly allocated string or NULL on error
 */
string_t *string_create_with_allocator (size_t initial_capacity,
                                        cutils_allocator_t *allocator);

/**
 * Creates a new string using the default allocator.
 *
 * @param initial_capacity Initial capacity of the string
 * @return Newly allocated string or NULL on error
 */
string_t *string_create (size_t initial_capacity);

/**
 * Creates a new string from a C string with the specified allocator.
 *
 * @param str C string to copy
 * @param allocator Allocator to use
 * @return Newly allocated string or NULL on error
 */
string_t *string_from_cstr_with_allocator (const char *str,
                                           cutils_allocator_t *allocator);

/**
 * Creates a new string from a C string using the default allocator.
 *
 * @param str C string to copy
 * @return Newly allocated string or NULL on error
 */
string_t *string_from_cstr (const char *str);

/**
 * Destroys string and frees all allocated memory.
 *
 * @param str String to destroy
 */
void string_destroy (string_t *str);

/**
 * Appends a C string to the string with timeout.
 *
 * @param str String to append to
 * @param append C string to append
 * @param timeout_ms Timeout in milliseconds
 * @return true if successful, false otherwise
 */
bool string_append_timeout (string_t *str, const char *append,
                            uint32_t timeout_ms);

/**
 * Appends a C string to the string.
 *
 * @param str String to append to
 * @param append C string to append
 * @return true if successful, false otherwise
 */
bool string_append (string_t *str, const char *append);

/**
 * Appends a character to the string with timeout.
 *
 * @param str String to append to
 * @param c Character to append
 * @param timeout_ms Timeout in milliseconds
 * @return true if successful, false otherwise
 */
bool string_append_char_timeout (string_t *str, char c, uint32_t timeout_ms);

/**
 * Appends a character to the string.
 *
 * @param str String to append to
 * @param c Character to append
 * @return true if successful, false otherwise
 */
bool string_append_char (string_t *str, char c);

/**
 * Gets the length of the string.
 *
 * @param str String to get length from
 * @return Length of the string
 */
size_t string_length (const string_t *str);

/**
 * Gets the capacity of the string.
 *
 * @param str String to get capacity from
 * @return Capacity of the string
 */
size_t string_capacity (const string_t *str);

/**
 * Checks if the string is empty.
 *
 * @param str String to check
 * @return true if empty, false otherwise
 */
bool string_is_empty (const string_t *str);

/**
 * Clears the string.
 *
 * @param str String to clear
 * @return true if successful, false otherwise
 */
bool string_clear (string_t *str);

/**
 * Gets the memory usage of the string.
 *
 * @param str String to get memory usage from
 * @return Memory usage in bytes
 */
size_t string_memory_usage (const string_t *str);

/**
 * Checks if an operation would succeed without actually performing it.
 *
 * @param str String to check
 * @param required_capacity Required capacity
 * @return true if operation would succeed, false otherwise
 */
bool string_can_perform_operation (const string_t *str,
                                   size_t required_capacity);

/**
 * Gets the last string operation error.
 *
 * @return Last error code
 */
string_result_t string_get_error (void);

/**
 * Gets the C string representation of the string.
 *
 * @param str String to get C string from
 * @return C string representation
 */
const char *string_cstr (const string_t *str);

/**
 * Finds a substring in the string.
 *
 * @param str String to search in
 * @param substr Substring to find
 * @return Index of the substring or SIZE_MAX if not found
 */
size_t string_find (const string_t *str, const char *substr);

/**
 * Finds a character in the string.
 *
 * @param str String to search in
 * @param c Character to find
 * @return Index of the character or SIZE_MAX if not found
 */
size_t string_find_char (const string_t *str, char c);

/**
 * Gets a substring of the string.
 *
 * @param str String to get substring from
 * @param start Start index
 * @param length Length of substring
 * @return New string containing the substring or NULL on error
 */
string_t *string_substring (const string_t *str, size_t start, size_t length);

/**
 * Compares two strings.
 *
 * @param str1 First string
 * @param str2 Second string
 * @return < 0 if str1 < str2, 0 if str1 == str2, > 0 if str1 > str2
 */
int string_compare (const string_t *str1, const string_t *str2);

/**
 * Compares a string with a C string.
 *
 * @param str1 String
 * @param str2 C string
 * @return < 0 if str1 < str2, 0 if str1 == str2, > 0 if str1 > str2
 */
int string_compare_cstr (const string_t *str1, const char *str2);

#endif // CUTILS_STRING_H
