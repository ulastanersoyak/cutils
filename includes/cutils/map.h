#ifndef CUTILS_MAP_H
#define CUTILS_MAP_H

#include "cutils/allocator.h"
#include "cutils/config.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct map_node
{
  void *key;
  void *value;
  struct map_node *left;
  struct map_node *right;
  struct map_node *parent;
  bool is_red;
} map_node_t;

typedef struct
{
  map_node_t *root;
  size_t size;
  size_t key_size;
  size_t value_size;
  cutils_allocator_t *allocator;
  int (*compare) (const void *a, const void *b);
} map_t;

typedef enum
{
  MAP_OK = 0,
  MAP_NULL_PTR = 1,
  MAP_NO_MEMORY = 2,
  MAP_INVALID_ARG = 3,
  MAP_KEY_EXISTS = 4,
  MAP_KEY_NOT_FOUND = 5,
  MAP_TIMEOUT = 6,
  MAP_OVERFLOW = 7
} map_result_t;

/**
 * Creates a new map with the specified allocator.
 *
 * @param key_size Size of key in bytes
 * @param value_size Size of value in bytes
 * @param compare Key comparison function
 * @param allocator Allocator to use
 * @return Newly allocated map or NULL on error
 */
map_t *map_create_with_allocator (size_t key_size, size_t value_size,
                                  int (*compare) (const void *a,
                                                  const void *b),
                                  cutils_allocator_t *allocator);

/**
 * Creates a new map using the default allocator.
 *
 * @param key_size Size of key in bytes
 * @param value_size Size of value in bytes
 * @param compare Key comparison function
 * @return Newly allocated map or NULL on error
 */
map_t *map_create (size_t key_size, size_t value_size,
                   int (*compare) (const void *a, const void *b));

/**
 * Destroys map and frees all allocated memory.
 *
 * @param map Map to destroy
 */
void map_destroy (map_t *map);

/**
 * Inserts a key-value pair into the map with timeout.
 *
 * @param map Map to insert into
 * @param key Key to insert
 * @param value Value to insert
 * @param timeout_ms Timeout in milliseconds
 * @return true if successful, false otherwise
 */
bool map_insert_timeout (map_t *map, const void *key, const void *value,
                         uint32_t timeout_ms);

/**
 * Inserts a key-value pair into the map.
 *
 * @param map Map to insert into
 * @param key Key to insert
 * @param value Value to insert
 * @return true if successful, false otherwise
 */
bool map_insert (map_t *map, const void *key, const void *value);

/**
 * Removes a key-value pair from the map.
 *
 * @param map Map to remove from
 * @param key Key to remove
 * @param out_value Output parameter to store the removed value
 * @return true if successful, false otherwise
 */
bool map_remove (map_t *map, const void *key, void *out_value);

/**
 * Gets a value from the map.
 *
 * @param map Map to get from
 * @param key Key to get
 * @param out_value Output parameter to store the value
 * @return true if successful, false otherwise
 */
bool map_get (const map_t *map, const void *key, void *out_value);

/**
 * Checks if a key exists in the map.
 *
 * @param map Map to check
 * @param key Key to check
 * @return true if key exists, false otherwise
 */
bool map_contains (const map_t *map, const void *key);

/**
 * Gets the number of key-value pairs in the map.
 *
 * @param map Map to get size from
 * @return Number of key-value pairs
 */
size_t map_size (const map_t *map);

/**
 * Checks if the map is empty.
 *
 * @param map Map to check
 * @return true if empty, false otherwise
 */
bool map_is_empty (const map_t *map);

/**
 * Clears all key-value pairs from the map.
 *
 * @param map Map to clear
 * @return true if successful, false otherwise
 */
bool map_clear (map_t *map);

/**
 * Gets the memory usage of the map.
 *
 * @param map Map to get memory usage from
 * @return Memory usage in bytes
 */
size_t map_memory_usage (const map_t *map);

/**
 * Checks if an operation would succeed without actually performing it.
 *
 * @param map Map to check
 * @param required_capacity Required capacity
 * @return true if operation would succeed, false otherwise
 */
bool map_can_perform_operation (const map_t *map, size_t required_capacity);

/**
 * Gets the last map operation error.
 *
 * @return Last error code
 */
map_result_t map_get_error (void);

// Iterator implementation
typedef struct
{
  map_t *map;
  map_node_t *current;
  bool is_valid;
} map_iterator_t;

/**
 * Creates an iterator starting from the first element.
 *
 * @param map Map to create iterator for
 * @return Iterator starting from the first element
 */
map_iterator_t map_begin (map_t *map);

/**
 * Creates an iterator starting from the last element.
 *
 * @param map Map to create iterator for
 * @return Iterator starting from the last element
 */
map_iterator_t map_end (map_t *map);

/**
 * Moves the iterator to the next element.
 *
 * @param it Iterator to move
 * @return true if successful, false if end of map
 */
bool map_iterator_next (map_iterator_t *it);

/**
 * Moves the iterator to the previous element.
 *
 * @param it Iterator to move
 * @return true if successful, false if beginning of map
 */
bool map_iterator_prev (map_iterator_t *it);

/**
 * Gets the current key-value pair.
 *
 * @param it Iterator to get from
 * @param out_key Output parameter to store the key
 * @param out_value Output parameter to store the value
 * @return true if successful, false if end of map
 */
bool map_iterator_get (map_iterator_t *it, void *out_key, void *out_value);

/**
 * Sets the current value.
 *
 * @param it Iterator to set for
 * @param value Value to set
 * @return true if successful, false if end of map
 */
bool map_iterator_set (map_iterator_t *it, const void *value);

/**
 * Checks if the iterator is valid.
 *
 * @param it Iterator to check
 * @return true if valid, false if end of map
 */
bool map_iterator_is_valid (const map_iterator_t *it);

#endif // CUTILS_MAP_H
