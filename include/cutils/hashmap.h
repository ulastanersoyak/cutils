#ifndef CUTILS_HASHMAP_H
#define CUTILS_HASHMAP_H

#include "cutils/allocator.h"
#include "cutils/config.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Hashes a key. Equal keys must hash to the same value */
typedef uint32_t (*hashmap_hash_fn) (const void *key);

/* Returns true if two keys are equal */
typedef bool (*hashmap_equal_fn) (const void *a, const void *b);

/**
 * Unordered key-value store with O(1) average lookups, using open
 * addressing with linear probing. Keys and values are copied in, like every
 * other container. Use map instead when you need the keys in order.
 */
typedef struct
{
  uint8_t *occupied; /* one byte per slot */
  void *keys;
  void *values;
  size_t capacity; /* always a power of two */
  size_t size;
  size_t key_size;
  size_t value_size;
  hashmap_hash_fn hash;
  hashmap_equal_fn equal;
  cutils_allocator_t *allocator;
} hashmap_t;

typedef enum
{
  HASHMAP_OK = 0,
  HASHMAP_NULL_PTR = 1,
  HASHMAP_NO_MEMORY = 2,
  HASHMAP_INVALID_ARG = 3,
  HASHMAP_KEY_EXISTS = 4,
  HASHMAP_KEY_NOT_FOUND = 5,
  HASHMAP_OVERFLOW = 6
} hashmap_result_t;

/**
 * Creates an empty hashmap.
 *
 * @param key_size Size of a key in bytes
 * @param value_size Size of a value in bytes
 * @param hash Hash function, or NULL to hash the key's bytes (fine for
 *             integers and structs without padding, not for pointers to
 *             strings)
 * @param equal Equality function, or NULL to compare the key's bytes
 * @param allocator Allocator to use
 * @return Newly allocated hashmap or NULL on error
 */
hashmap_t *hashmap_create_with_allocator (size_t key_size, size_t value_size,
                                          hashmap_hash_fn hash,
                                          hashmap_equal_fn equal,
                                          cutils_allocator_t *allocator);

/**
 * Creates an empty hashmap using the default allocator.
 */
hashmap_t *hashmap_create (size_t key_size, size_t value_size,
                           hashmap_hash_fn hash, hashmap_equal_fn equal);

/**
 * Destroys a hashmap.
 *
 * @param map Hashmap to destroy
 */
void hashmap_destroy (hashmap_t *map);

/**
 * Inserts a key-value pair.
 *
 * @return true if successful, false otherwise
 * @note Sets error to HASHMAP_KEY_EXISTS if the key is already present
 */
bool hashmap_insert (hashmap_t *map, const void *key, const void *value);

/**
 * Inserts a key-value pair, or overwrites the value if the key is present.
 *
 * @return true if successful, false otherwise
 */
bool hashmap_insert_or_assign (hashmap_t *map, const void *key,
                               const void *value);

/**
 * Copies the value stored for key into out_value.
 *
 * @return true if found, false otherwise
 * @note Sets error to HASHMAP_KEY_NOT_FOUND if the key isn't present
 */
bool hashmap_get (const hashmap_t *map, const void *key, void *out_value);

/**
 * Gets a pointer to the value stored for key, to read or change it in
 * place.
 *
 * @return Pointer to the value, or NULL if the key isn't present. Invalidated
 *         by any insert or remove
 */
void *hashmap_get_ptr (hashmap_t *map, const void *key);

/**
 * Checks whether key is present.
 */
bool hashmap_contains (const hashmap_t *map, const void *key);

/**
 * Removes key and copies its value into out_value, which may be NULL.
 *
 * @return true if the key was present, false otherwise
 */
bool hashmap_remove (hashmap_t *map, const void *key, void *out_value);

/**
 * Removes every entry, keeping the allocated capacity.
 */
bool hashmap_clear (hashmap_t *map);

/**
 * Gets the number of entries.
 */
size_t hashmap_size (const hashmap_t *map);

/**
 * Steps through the entries in no particular order. Start with *cursor set
 * to 0 and call until it returns false. out_key and out_value may be NULL.
 * The map must not be changed during the walk.
 *
 *   size_t cursor = 0;
 *   while (hashmap_next (map, &cursor, &key, &value)) { ... }
 *
 * @return true if an entry was copied out, false when there are no more
 */
bool hashmap_next (const hashmap_t *map, size_t *cursor, void *out_key,
                   void *out_value);

/**
 * Gets the last hashmap operation error.
 */
hashmap_result_t hashmap_get_error (void);

#endif // CUTILS_HASHMAP_H
