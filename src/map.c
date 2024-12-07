#include "cutils/map.h"
#include <stdlib.h>
#include <string.h>

static map_result_t last_error = MAP_OK;

[[nodiscard]] map_result_t
map_get_error (void)
{
  return last_error;
}

[[nodiscard]] static size_t
next_power_of_2 (size_t n)
{
  n--;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  n |= n >> 32;
  n++;
  return n;
}

[[nodiscard]] map_t *
map_create (size_t key_size, size_t value_size, size_t initial_capacity,
            map_hash_fn hash_fn, map_eq_fn eq_fn, map_free_fn key_free,
            map_free_fn value_free)
{
  if (hash_fn == NULL || eq_fn == NULL)
    {
      last_error = MAP_NULL_PTR;
      return NULL;
    }

  map_t *map = (map_t *)malloc (sizeof (map_t));
  if (map == NULL)
    {
      last_error = MAP_NO_MEMORY;
      return NULL;
    }

  initial_capacity = next_power_of_2 (initial_capacity);
  map->buckets
      = (map_entry_t **)calloc (initial_capacity, sizeof (map_entry_t *));
  if (map->buckets == NULL)
    {
      free (map);
      last_error = MAP_NO_MEMORY;
      return NULL;
    }

  map->bucket_count = initial_capacity;
  map->size = 0;
  map->key_size = key_size;
  map->value_size = value_size;
  map->hash_fn = hash_fn;
  map->eq_fn = eq_fn;
  map->key_free = key_free;
  map->value_free = value_free;

  return map;
}

static map_entry_t *
create_entry (const map_t *map, const void *key, const void *value)
{
  map_entry_t *entry = (map_entry_t *)malloc (sizeof (map_entry_t));
  if (entry == NULL)
    {
      return NULL;
    }

  entry->key = malloc (map->key_size);
  if (entry->key == NULL)
    {
      free (entry);
      return NULL;
    }

  entry->value = malloc (map->value_size);
  if (entry->value == NULL)
    {
      free (entry->key);
      free (entry);
      return NULL;
    }

  memcpy (entry->key, key, map->key_size);
  memcpy (entry->value, value, map->value_size);
  entry->next = NULL;

  return entry;
}

static void
free_entry (map_t *map, map_entry_t *entry)
{
  if (map->key_free != NULL)
    {
      map->key_free (entry->key);
    }
  if (map->value_free != NULL)
    {
      map->value_free (entry->value);
    }
  free (entry->key);
  free (entry->value);
  free (entry);
}

static map_result_t
resize_map (map_t *map)
{
  size_t new_capacity = map->bucket_count * 2;
  map_entry_t **new_buckets
      = (map_entry_t **)calloc (new_capacity, sizeof (map_entry_t *));
  if (new_buckets == NULL)
    {
      return MAP_NO_MEMORY;
    }

  for (size_t i = 0; i < map->bucket_count; i++)
    {
      map_entry_t *entry = map->buckets[i];
      while (entry != NULL)
        {
          map_entry_t *next = entry->next;
          size_t new_index = map->hash_fn (entry->key) % new_capacity;
          entry->next = new_buckets[new_index];
          new_buckets[new_index] = entry;
          entry = next;
        }
    }

  free (map->buckets);
  map->buckets = new_buckets;
  map->bucket_count = new_capacity;

  return MAP_OK;
}

map_result_t
map_insert (map_t *map, const void *key, const void *value)
{
  if (map == NULL || key == NULL || value == NULL)
    {
      last_error = MAP_NULL_PTR;
      return MAP_NULL_PTR;
    }

  float load_factor = (float)map->size / (float)map->bucket_count;
  if (load_factor >= 0.75f)
    {
      map_result_t result = resize_map (map);
      if (result != MAP_OK)
        {
          return result; // error is set by resize_map
        }
    }

  size_t index = map->hash_fn (key) % map->bucket_count;
  map_entry_t *entry = map->buckets[index];

  while (entry != NULL)
    {
      if (map->eq_fn (entry->key, key))
        {
          last_error = MAP_DUPLICATE_KEY;
          return MAP_DUPLICATE_KEY;
        }
      entry = entry->next;
    }

  map_entry_t *new_entry = create_entry (map, key, value);
  if (new_entry == NULL)
    {
      last_error = MAP_NO_MEMORY;
      return MAP_NO_MEMORY;
    }

  new_entry->next = map->buckets[index];
  map->buckets[index] = new_entry;
  map->size++;

  last_error = MAP_OK;
  return MAP_OK;
}

map_result_t
map_get (const map_t *map, const void *key, void *value_out)
{
  if (map == NULL || key == NULL || value_out == NULL)
    {
      last_error = MAP_NULL_PTR;
      return MAP_NULL_PTR;
    }

  size_t index = map->hash_fn (key) % map->bucket_count;
  map_entry_t *entry = map->buckets[index];

  while (entry != NULL)
    {
      if (map->eq_fn (entry->key, key))
        {
          memcpy (value_out, entry->value, map->value_size);
          last_error = MAP_OK;
          return MAP_OK;
        }
      entry = entry->next;
    }

  last_error = MAP_KEY_NOT_FOUND;
  return MAP_KEY_NOT_FOUND;
}

map_result_t
map_remove (map_t *map, const void *key)
{
  if (map == NULL || key == NULL)
    {
      last_error = MAP_NULL_PTR;
      return MAP_NULL_PTR;
    }

  size_t index = map->hash_fn (key) % map->bucket_count;
  map_entry_t *entry = map->buckets[index];
  map_entry_t *prev = NULL;

  while (entry != NULL)
    {
      if (map->eq_fn (entry->key, key))
        {
          if (prev == NULL)
            {
              map->buckets[index] = entry->next;
            }
          else
            {
              prev->next = entry->next;
            }
          free_entry (map, entry);
          map->size--;
          last_error = MAP_OK;
          return MAP_OK;
        }
      prev = entry;
      entry = entry->next;
    }

  last_error = MAP_KEY_NOT_FOUND;
  return MAP_KEY_NOT_FOUND;
}

size_t
map_size (const map_t *map)
{
  if (map == NULL)
    {
      return 0;
    }
  return map->size;
}

bool
map_contains (const map_t *map, const void *key)
{
  if (map == NULL || key == NULL)
    {
      last_error = MAP_NULL_PTR;
      return false;
    }

  size_t index = map->hash_fn (key) % map->bucket_count;
  map_entry_t *entry = map->buckets[index];

  while (entry != NULL)
    {
      if (map->eq_fn (entry->key, key))
        {
          last_error = MAP_OK;
          return true;
        }
      entry = entry->next;
    }

  last_error = MAP_OK;
  return false;
}

void
map_destroy (map_t *map)
{
  if (map == NULL)
    {
      return;
    }

  for (size_t i = 0; i < map->bucket_count; i++)
    {
      map_entry_t *entry = map->buckets[i];
      while (entry != NULL)
        {
          map_entry_t *next = entry->next;
          free_entry (map, entry);
          entry = next;
        }
    }

  free (map->buckets);
  free (map);
}
