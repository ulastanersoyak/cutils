#ifndef CUTILS_MAP_H
#define CUTILS_MAP_H

#include <stdbool.h>
#include <stddef.h>

typedef enum
{
  MAP_OK,
  MAP_NULL_PTR,
  MAP_NO_MEMORY,
  MAP_KEY_NOT_FOUND,
  MAP_DUPLICATE_KEY,
  MAP_INVALID_ARG
} map_result_t;

typedef size_t (*map_hash_fn) (const void *key);
typedef bool (*map_eq_fn) (const void *key1, const void *key2);
typedef void (*map_free_fn) (void *ptr);

typedef struct map_entry
{
  void *key;
  void *value;
  struct map_entry *next;
} map_entry_t;

typedef struct
{
  map_entry_t **buckets;
  size_t bucket_count;
  size_t size;
  size_t key_size;
  size_t value_size;
  map_hash_fn hash_fn;
  map_eq_fn eq_fn;
  map_free_fn key_free;
  map_free_fn value_free;
} map_t;

/**
 * Gets the last map operation error.
 *
 * @return Last error code
 */
[[nodiscard]] map_result_t map_get_error (void);

/**
 * Creates a new empty map.
 *
 * @param key_size Size of keys in bytes
 * @param value_size Size of values in bytes
 * @param initial_capacity Initial number of buckets (rounded to power of 2)
 * @param hash_fn Function to hash keys
 * @param eq_fn Function to compare keys for equality
 * @param key_free Optional function to free keys (can be NULL)
 * @param value_free Optional function to free values (can be NULL)
 * @return Newly allocated map or NULL on error
 * @note Sets error to MAP_NULL_PTR if hash_fn or eq_fn is NULL
 * @note Sets error to MAP_NO_MEMORY if memory allocation fails
 */
[[nodiscard]] map_t *map_create (size_t key_size, size_t value_size,
                                 size_t initial_capacity, map_hash_fn hash_fn,
                                 map_eq_fn eq_fn, map_free_fn key_free,
                                 map_free_fn value_free);

/**
 * Inserts a key-value pair into the map.
 *
 * @param map Map to insert into
 * @param key Pointer to key to insert
 * @param value Pointer to value to insert
 * @return MAP_OK on success, error code on failure
 * @note Sets error to MAP_NULL_PTR if map, key or value is NULL
 * @note Sets error to MAP_NO_MEMORY if memory allocation fails
 * @note Sets error to MAP_DUPLICATE_KEY if key already exists
 */
map_result_t map_insert (map_t *map, const void *key, const void *value);

/**
 * Gets a value from the map by key.
 *
 * @param map Map to get from
 * @param key Key to look up
 * @param value_out Pointer to store found value
 * @return MAP_OK on success, error code on failure
 * @note Sets error to MAP_NULL_PTR if map, key or value_out is NULL
 * @note Sets error to MAP_KEY_NOT_FOUND if key doesn't exist
 */
map_result_t map_get (const map_t *map, const void *key, void *value_out);

/**
 * Removes a key-value pair from the map.
 *
 * @param map Map to remove from
 * @param key Key to remove
 * @return MAP_OK on success, error code on failure
 * @note Sets error to MAP_NULL_PTR if map or key is NULL
 * @note Sets error to MAP_KEY_NOT_FOUND if key doesn't exist
 */
map_result_t map_remove (map_t *map, const void *key);

/**
 * Gets the number of key-value pairs in the map.
 *
 * @param map Map to get size of
 * @return Number of pairs, 0 if map is NULL
 */
size_t map_size (const map_t *map);

/**
 * Checks if map contains a key.
 *
 * @param map Map to check
 * @param key Key to look for
 * @return true if key exists, false otherwise
 * @note Sets error to MAP_NULL_PTR if map or key is NULL
 */
bool map_contains (const map_t *map, const void *key);

/**
 * Frees all memory associated with a map.
 *
 * @param map Map to destroy
 */
void map_destroy (map_t *map);

#endif // CUTILS_MAP_H
