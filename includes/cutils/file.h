#ifndef CUTILS_FILE_H
#define CUTILS_FILE_H

#include <stdbool.h>
#include <stddef.h>

typedef enum
{
  FILE_OK,
  FILE_NOT_FOUND,
  FILE_NO_PERMISSION,
  FILE_IO_ERROR,
  FILE_NULL_PTR,
  FILE_NO_MEMORY
} file_result_t;

/**
 * Gets the last file operation error.
 *
 * @return Last error code
 */
[[nodiscard]] file_result_t file_get_error (void);

/**
 * Checks if a file exists at the given path.
 *
 * @param path Path to check
 * @return true if file exists, false otherwise
 * @note Sets error to FILE_NULL_PTR if path is NULL
 */
[[nodiscard]] bool file_exists (const char *path);

/**
 * Gets the size of a file in bytes.
 *
 * @param path Path to file
 * @return Size of file in bytes, 0 on error
 * @note Sets error to FILE_NULL_PTR if path is NULL
 * @note Sets error to FILE_NOT_FOUND if file doesn't exist
 */
[[nodiscard]] size_t file_size (const char *path);

/**
 * Reads entire file into a null-terminated string.
 *
 * @param path Path to file to read
 * @return Malloc'd string containing file contents, NULL on error
 * @note Sets error to FILE_NULL_PTR if path is NULL
 * @note Sets error to FILE_NOT_FOUND if file doesn't exist
 * @note Sets error to FILE_NO_MEMORY if allocation fails
 * @note Caller must free returned string
 */
[[nodiscard]] char *file_read_all (const char *path);

/**
 * Reads file into an array of lines.
 *
 * @param path Path to file to read
 * @param line_count Pointer to store number of lines
 * @return Malloc'd array of malloc'd strings, NULL on error
 * @note Sets error to FILE_NULL_PTR if any parameter is NULL
 * @note Sets error to FILE_NOT_FOUND if file doesn't exist
 * @note Sets error to FILE_NO_MEMORY if allocation fails
 * @note Caller must free each string and the array
 */
[[nodiscard]] char **file_read_lines (const char *path, size_t *line_count);

/**
 * Reads binary data from file into buffer.
 *
 * @param path Path to file to read
 * @param buffer Buffer to store data
 * @param size Number of bytes to read
 * @return true if successful, false otherwise
 * @note Sets error to FILE_NULL_PTR if any parameter is NULL
 * @note Sets error to FILE_NOT_FOUND if file doesn't exist
 * @note Sets error to FILE_IO_ERROR if read fails
 */
[[nodiscard]] bool file_read_bytes (const char *path, void *buffer,
                                    size_t size);

#endif // CUTILS_FILE_H
