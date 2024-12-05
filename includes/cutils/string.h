#ifndef CUTILS_STRING_H
#define CUTILS_STRING_H

#include <stddef.h>
#include <stdint.h>

typedef enum
{
  STRING_OK,
  STRING_NULL_PTR,
  STRING_INVALID_ARG,
  STRING_NO_MEMORY,
  STRING_OVERFLOW
} string_result_t;

typedef struct
{
  char *data;
  size_t len;
} string_t;

[[nodiscard]] string_result_t string_get_error (void);

/**
 * Creates a new string of specified length from given data.
 *
 * @param data Pointer to the data to copy into the new string
 * @param len Length of the data in bytes
 * @return Newly allocated string or NULL on error
 * @note Sets error to STRING_NULL_PTR if data is NULL
 * @note Sets error to STRING_OVERFLOW if len is too large
 * @note Sets error to STRING_NO_MEMORY if memory allocation fails
 */
[[nodiscard]] string_t *string_create (const char *data, size_t len);

/**
 * Creates a new string from a null-terminated C string.
 *
 * @param str Pointer to null-terminated string
 * @return Newly allocated string or NULL on error
 * @note Sets error to STRING_NULL_PTR if str is NULL
 */
[[nodiscard]] string_t *string_from_cstr (const char *str);

/**
 * Creates a deep copy of a string.
 *
 * @param str String to copy
 * @return Newly allocated copy of string or NULL on error
 * @note Sets error to STRING_NULL_PTR if str is NULL
 * @note Sets error to STRING_INVALID_ARG if str->data is NULL
 */
[[nodiscard]] string_t *string_copy (const string_t *str);

/**
 * Takes ownership of string data and creates new string.
 * Original string is invalidated (data set to NULL).
 *
 * @param str String to move from
 * @return New string containing moved data or NULL on error
 * @note Sets error to STRING_NULL_PTR if str is NULL
 * @note Sets error to STRING_INVALID_ARG if str->data is NULL
 */
[[nodiscard]] string_t *string_move (string_t *str);

/**
 * Frees all memory associated with string.
 *
 * @param str String to destroy
 * @note Sets error to STRING_NULL_PTR if str is NULL
 */
void string_destroy (string_t *str);

/**
 * Checks if two strings have identical content.
 *
 * @param first First string to compare
 * @param second Second string to compare
 * @return true if strings are equal, false otherwise
 * @note Sets error to STRING_NULL_PTR if either parameter is NULL
 * @note Sets error to STRING_INVALID_ARG if any string data is NULL
 */
[[nodiscard]] bool string_equals (const string_t *first,
                                  const string_t *second);

/**
 * Checks if string starts with given prefix.
 *
 * @param str String to check
 * @param prefix Prefix to look for
 * @return true if str starts with prefix, false otherwise
 * @note Sets error to STRING_NULL_PTR if either parameter is NULL
 * @note Sets error to STRING_INVALID_ARG if any string data is NULL
 */
[[nodiscard]] bool string_starts_with (const string_t *str,
                                       const string_t *prefix);

/**
 * Checks if string ends with given suffix.
 *
 * @param str String to check
 * @param suffix Suffix to look for
 * @return true if str ends with suffix, false otherwise
 * @note Sets error to STRING_NULL_PTR if either parameter is NULL
 * @note Sets error to STRING_INVALID_ARG if any string data is NULL
 */
[[nodiscard]] bool string_ends_with (const string_t *str,
                                     const string_t *suffix);

/**
 * Creates new string with whitespace removed from both ends.
 *
 * @param str String to trim
 * @return New trimmed string or NULL on error
 * @note Sets error to STRING_NULL_PTR if str is NULL
 * @note Sets error to STRING_INVALID_ARG if str->data is NULL
 */
[[nodiscard]] string_t *string_trim (const string_t *str);

/**
 * Creates new string with whitespace removed from start.
 *
 * @param str String to trim
 * @return New left-trimmed string or NULL on error
 * @note Sets error to STRING_NULL_PTR if str is NULL
 * @note Sets error to STRING_INVALID_ARG if str->data is NULL
 */
[[nodiscard]] string_t *string_trim_left (const string_t *str);

/**
 * Creates new string with whitespace removed from end.
 *
 * @param str String to trim
 * @return New right-trimmed string or NULL on error
 * @note Sets error to STRING_NULL_PTR if str is NULL
 * @note Sets error to STRING_INVALID_ARG if str->data is NULL
 */
[[nodiscard]] string_t *string_trim_right (const string_t *str);

/**
 * Creates new string with all characters converted to uppercase.
 *
 * @param str String to convert
 * @return New uppercase string or NULL on error
 * @note Sets error to STRING_NULL_PTR if str is NULL
 * @note Sets error to STRING_INVALID_ARG if str->data is NULL
 */
[[nodiscard]] string_t *string_to_upper (const string_t *str);

/**
 * Creates new string with all characters converted to lowercase.
 *
 * @param str String to convert
 * @return New lowercase string or NULL on error
 * @note Sets error to STRING_NULL_PTR if str is NULL
 * @note Sets error to STRING_INVALID_ARG if str->data is NULL
 */
[[nodiscard]] string_t *string_to_lower (const string_t *str);

/**
 * Creates new string containing specified portion of input string.
 *
 * @param str String to get substring from
 * @param start Starting position of substring
 * @param length Length of substring
 * @return New substring or NULL on error
 * @note Sets error to STRING_NULL_PTR if str is NULL
 * @note Sets error to STRING_INVALID_ARG if str->data is NULL or indexes
 * invalid
 */
[[nodiscard]] string_t *string_substring (const string_t *str, size_t start,
                                          size_t length);

/**
 * Creates new string by concatenating two strings.
 *
 * @param first First string
 * @param second Second string
 * @return New concatenated string or NULL on error
 * @note Sets error to STRING_NULL_PTR if either parameter is NULL
 * @note Sets error to STRING_INVALID_ARG if any string data is NULL
 * @note Sets error to STRING_OVERFLOW if combined length too large
 */
[[nodiscard]] string_t *string_concat (const string_t *first,
                                       const string_t *second);

/**
 * Finds first occurrence of pattern in string.
 *
 * @param str String to search in
 * @param pattern Pattern to search for
 * @return Index of first occurrence or SIZE_MAX if not found
 * @note Sets error to STRING_NULL_PTR if either parameter is NULL
 * @note Sets error to STRING_INVALID_ARG if any string data is NULL
 */
[[nodiscard]] size_t string_find (const string_t *str,
                                  const string_t *pattern);

/**
 * Finds last occurrence of pattern in string.
 *
 * @param str String to search in
 * @param pattern Pattern to search for
 * @return Index of last occurrence or SIZE_MAX if not found
 * @note Sets error to STRING_NULL_PTR if either parameter is NULL
 * @note Sets error to STRING_INVALID_ARG if any string data is NULL
 */
[[nodiscard]] size_t string_find_last (const string_t *str,
                                       const string_t *pattern);

/**
 * Checks if string contains substring.
 *
 * @param str String to search in
 * @param substr Substring to search for
 * @return true if substring found, false otherwise
 * @note Sets error to STRING_NULL_PTR if either parameter is NULL
 * @note Sets error to STRING_INVALID_ARG if any string data is NULL
 */
[[nodiscard]] bool string_contains (const string_t *str,
                                    const string_t *substr);

/**
 * Creates new string with characters in reverse order.
 *
 * @param str String to reverse
 * @return New reversed string or NULL on error
 * @note Sets error to STRING_NULL_PTR if str is NULL
 * @note Sets error to STRING_INVALID_ARG if str->data is NULL
 */
[[nodiscard]] string_t *string_reverse (const string_t *str);

/**
 * Converts string to 64-bit integer.
 *
 * @param str String to convert
 * @param success Pointer to bool to indicate success/failure
 * @return Converted integer value, 0 on error
 * @note Sets error to STRING_NULL_PTR if str is NULL
 * @note Sets error to STRING_INVALID_ARG if str->data is NULL or invalid
 * format
 * @note Sets error to STRING_OVERFLOW if value out of range
 */
[[nodiscard]] int64_t string_to_int (const string_t *str, bool *success);

/**
 * Creates new string from 64-bit integer.
 *
 * @param value Integer value to convert
 * @return New string containing number or NULL on error
 * @note Sets error to STRING_OVERFLOW if conversion fails
 */
[[nodiscard]] string_t *string_from_int (int64_t value);

#endif
