#include "cutils/hashmap.h"
#include "cutils/config.h"
#include "internal/capacity.h"

#include <stdint.h>
#include <string.h>
#include <threads.h>

#define HASHMAP_MIN_CAPACITY 16

static thread_local hashmap_result_t g_last_error = HASHMAP_OK;

static char *
key_at (const hashmap_t *map, size_t slot)
{
  return (char *)map->keys + (slot * map->key_size);
}

static char *
value_at (const hashmap_t *map, size_t slot)
{
  return (char *)map->values + (slot * map->value_size);
}

// fnv-1a over the key's bytes, used when no hash function is given
static uint32_t
hash_bytes (const void *key, size_t size)
{
  const unsigned char *bytes = (const unsigned char *)key;
  uint32_t hash = 2166136261u;
  for (size_t i = 0; i < size; i++)
    {
      hash ^= bytes[i];
      hash *= 16777619u;
    }
  return hash;
}

static size_t
home_slot (const hashmap_t *map, const void *key)
{
  uint32_t hash
      = map->hash != NULL ? map->hash (key) : hash_bytes (key, map->key_size);
  return (size_t)hash & (map->capacity - 1);
}

static bool
keys_equal (const hashmap_t *map, const void *a, const void *b)
{
  return map->equal != NULL ? map->equal (a, b)
                            : memcmp (a, b, map->key_size) == 0;
}

// returns the slot holding key, or SIZE_MAX
static size_t
find_slot (const hashmap_t *map, const void *key)
{
  size_t mask = map->capacity - 1;
  for (size_t slot = home_slot (map, key);; slot = (slot + 1) & mask)
    {
      if (!map->occupied[slot])
        {
          return SIZE_MAX;
        }
      if (keys_equal (map, key_at (map, slot), key))
        {
          return slot;
        }
    }
}

typedef struct
{
  uint8_t *occupied;
  void *keys;
  void *values;
} tables_t;

static void
free_tables (cutils_allocator_t *allocator, tables_t *tables)
{
  cutils_deallocate (allocator, tables->occupied);
  cutils_deallocate (allocator, tables->keys);
  cutils_deallocate (allocator, tables->values);
}

static bool
alloc_tables (cutils_allocator_t *allocator, size_t capacity, size_t key_size,
              size_t value_size, tables_t *tables)
{
  size_t key_bytes;
  size_t value_bytes;
  if (!cutils_array_bytes (capacity, key_size, &key_bytes)
      || !cutils_array_bytes (capacity, value_size, &value_bytes))
    {
      g_last_error = HASHMAP_OVERFLOW;
      return false;
    }

  tables->occupied
      = cutils_allocate_aligned (allocator, capacity, CUTILS_ALIGNMENT);
  tables->keys
      = cutils_allocate_aligned (allocator, key_bytes, CUTILS_ALIGNMENT);
  tables->values
      = cutils_allocate_aligned (allocator, value_bytes, CUTILS_ALIGNMENT);
  if (tables->occupied == NULL || tables->keys == NULL
      || tables->values == NULL)
    {
      free_tables (allocator, tables);
      g_last_error = HASHMAP_NO_MEMORY;
      return false;
    }

  memset (tables->occupied, 0, capacity);
  return true;
}

// places an entry known not to be present, without checking the load
static void
place (hashmap_t *map, const void *key, const void *value)
{
  size_t mask = map->capacity - 1;
  size_t slot = home_slot (map, key);
  while (map->occupied[slot])
    {
      slot = (slot + 1) & mask;
    }
  map->occupied[slot] = 1;
  memcpy (key_at (map, slot), key, map->key_size);
  memcpy (value_at (map, slot), value, map->value_size);
  map->size++;
}

static bool
grow (hashmap_t *map)
{
  if (map->capacity > SIZE_MAX / 2)
    {
      g_last_error = HASHMAP_OVERFLOW;
      return false;
    }

  hashmap_t old = *map;
  tables_t tables;
  if (!alloc_tables (map->allocator, map->capacity * 2, map->key_size,
                     map->value_size, &tables))
    {
      return false;
    }

  map->occupied = tables.occupied;
  map->keys = tables.keys;
  map->values = tables.values;
  map->capacity *= 2;
  map->size = 0;
  for (size_t slot = 0; slot < old.capacity; slot++)
    {
      if (old.occupied[slot])
        {
          place (map, key_at (&old, slot), value_at (&old, slot));
        }
    }

  tables_t old_tables = { old.occupied, old.keys, old.values };
  free_tables (map->allocator, &old_tables);
  return true;
}

hashmap_t *
hashmap_create_with_allocator (size_t key_size, size_t value_size,
                               hashmap_hash_fn hash, hashmap_equal_fn equal,
                               cutils_allocator_t *allocator)
{
  g_last_error = HASHMAP_OK;

  if (key_size == 0 || value_size == 0 || allocator == NULL)
    {
      g_last_error = HASHMAP_INVALID_ARG;
      return NULL;
    }

  hashmap_t *map = cutils_allocate_aligned (allocator, sizeof (hashmap_t),
                                            CUTILS_ALIGNMENT);
  if (map == NULL)
    {
      g_last_error = HASHMAP_NO_MEMORY;
      return NULL;
    }

  tables_t tables;
  if (!alloc_tables (allocator, HASHMAP_MIN_CAPACITY, key_size, value_size,
                     &tables))
    {
      cutils_deallocate (allocator, map);
      return NULL;
    }

  map->occupied = tables.occupied;
  map->keys = tables.keys;
  map->values = tables.values;
  map->capacity = HASHMAP_MIN_CAPACITY;
  map->size = 0;
  map->key_size = key_size;
  map->value_size = value_size;
  map->hash = hash;
  map->equal = equal;
  map->allocator = allocator;
  return map;
}

hashmap_t *
hashmap_create (size_t key_size, size_t value_size, hashmap_hash_fn hash,
                hashmap_equal_fn equal)
{
  return hashmap_create_with_allocator (key_size, value_size, hash, equal,
                                        cutils_default_allocator ());
}

void
hashmap_destroy (hashmap_t *map)
{
  g_last_error = HASHMAP_OK;

  if (map == NULL)
    {
      g_last_error = HASHMAP_NULL_PTR;
      return;
    }

  tables_t tables = { map->occupied, map->keys, map->values };
  free_tables (map->allocator, &tables);
  cutils_deallocate (map->allocator, map);
}

static bool
insert (hashmap_t *map, const void *key, const void *value, bool assign)
{
  g_last_error = HASHMAP_OK;

  if (map == NULL || key == NULL || value == NULL)
    {
      g_last_error = HASHMAP_NULL_PTR;
      return false;
    }

  size_t slot = find_slot (map, key);
  if (slot != SIZE_MAX)
    {
      if (!assign)
        {
          g_last_error = HASHMAP_KEY_EXISTS;
          return false;
        }
      memcpy (value_at (map, slot), value, map->value_size);
      return true;
    }

  // keep the load at or below 3/4 so probe runs stay short
  if ((map->size + 1) * 4 > map->capacity * 3 && !grow (map))
    {
      return false;
    }

  place (map, key, value);
  return true;
}

bool
hashmap_insert (hashmap_t *map, const void *key, const void *value)
{
  return insert (map, key, value, false);
}

bool
hashmap_insert_or_assign (hashmap_t *map, const void *key, const void *value)
{
  return insert (map, key, value, true);
}

bool
hashmap_get (const hashmap_t *map, const void *key, void *out_value)
{
  g_last_error = HASHMAP_OK;

  if (map == NULL || key == NULL || out_value == NULL)
    {
      g_last_error = HASHMAP_NULL_PTR;
      return false;
    }

  size_t slot = find_slot (map, key);
  if (slot == SIZE_MAX)
    {
      g_last_error = HASHMAP_KEY_NOT_FOUND;
      return false;
    }

  memcpy (out_value, value_at (map, slot), map->value_size);
  return true;
}

void *
hashmap_get_ptr (hashmap_t *map, const void *key)
{
  g_last_error = HASHMAP_OK;

  if (map == NULL || key == NULL)
    {
      g_last_error = HASHMAP_NULL_PTR;
      return NULL;
    }

  size_t slot = find_slot (map, key);
  if (slot == SIZE_MAX)
    {
      g_last_error = HASHMAP_KEY_NOT_FOUND;
      return NULL;
    }

  return value_at (map, slot);
}

bool
hashmap_contains (const hashmap_t *map, const void *key)
{
  g_last_error = HASHMAP_OK;

  if (map == NULL || key == NULL)
    {
      g_last_error = HASHMAP_NULL_PTR;
      return false;
    }

  return find_slot (map, key) != SIZE_MAX;
}

// true if home lies cyclically in (start, end]
static bool
in_cyclic_range (size_t home, size_t start, size_t end)
{
  return start <= end ? (start < home && home <= end)
                      : (start < home || home <= end);
}

bool
hashmap_remove (hashmap_t *map, const void *key, void *out_value)
{
  g_last_error = HASHMAP_OK;

  if (map == NULL || key == NULL)
    {
      g_last_error = HASHMAP_NULL_PTR;
      return false;
    }

  size_t hole = find_slot (map, key);
  if (hole == SIZE_MAX)
    {
      g_last_error = HASHMAP_KEY_NOT_FOUND;
      return false;
    }

  if (out_value != NULL)
    {
      memcpy (out_value, value_at (map, hole), map->value_size);
    }

  // backward shift deletion: pull later entries of the probe run into the
  // hole unless that would move them before their home slot. leaves no
  // tombstones, so lookups never slow down after many removes
  size_t mask = map->capacity - 1;
  map->occupied[hole] = 0;
  for (size_t next = (hole + 1) & mask; map->occupied[next];
       next = (next + 1) & mask)
    {
      size_t home = home_slot (map, key_at (map, next));
      if (in_cyclic_range (home, hole, next))
        {
          continue;
        }
      memcpy (key_at (map, hole), key_at (map, next), map->key_size);
      memcpy (value_at (map, hole), value_at (map, next), map->value_size);
      map->occupied[hole] = 1;
      map->occupied[next] = 0;
      hole = next;
    }

  map->size--;
  return true;
}

bool
hashmap_clear (hashmap_t *map)
{
  g_last_error = HASHMAP_OK;

  if (map == NULL)
    {
      g_last_error = HASHMAP_NULL_PTR;
      return false;
    }

  memset (map->occupied, 0, map->capacity);
  map->size = 0;
  return true;
}

size_t
hashmap_size (const hashmap_t *map)
{
  if (map == NULL)
    {
      g_last_error = HASHMAP_NULL_PTR;
      return 0;
    }
  return map->size;
}

bool
hashmap_next (const hashmap_t *map, size_t *cursor, void *out_key,
              void *out_value)
{
  if (map == NULL || cursor == NULL)
    {
      g_last_error = HASHMAP_NULL_PTR;
      return false;
    }

  for (size_t slot = *cursor; slot < map->capacity; slot++)
    {
      if (map->occupied[slot])
        {
          if (out_key != NULL)
            {
              memcpy (out_key, key_at (map, slot), map->key_size);
            }
          if (out_value != NULL)
            {
              memcpy (out_value, value_at (map, slot), map->value_size);
            }
          *cursor = slot + 1;
          return true;
        }
    }

  *cursor = map->capacity;
  return false;
}

hashmap_result_t
hashmap_get_error (void)
{
  return g_last_error;
}
