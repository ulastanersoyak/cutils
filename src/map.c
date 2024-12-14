#include "cutils/map.h"
#include <stdlib.h>
#include <string.h>

static map_result_t g_last_error = MAP_OK;

[[nodiscard]] map_result_t
map_get_error (void)
{
  return g_last_error;
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
  g_last_error = MAP_OK;

  if (hash_fn == NULL || eq_fn == NULL)
    {
      g_last_error = MAP_NULL_PTR;
      return NULL;
    }

  map_t *map = (map_t *)malloc (sizeof (map_t));
  if (map == NULL)
    {
      g_last_error = MAP_NO_MEMORY;
      return NULL;
    }

  initial_capacity = next_power_of_2 (initial_capacity);
  map->buckets
      = (map_entry_t **)calloc (initial_capacity, sizeof (map_entry_t *));
  if (map->buckets == NULL)
    {
      free (map);
      g_last_error = MAP_NO_MEMORY;
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
  g_last_error = MAP_OK;

  map_entry_t *entry = (map_entry_t *)malloc (sizeof (map_entry_t));
  if (entry == NULL)
    {
      g_last_error = MAP_NO_MEMORY;
      return NULL;
    }

  entry->key = malloc (map->key_size);
  if (entry->key == NULL)
    {
      free (entry);
      g_last_error = MAP_NO_MEMORY;
      return NULL;
    }

  entry->value = malloc (map->value_size);
  if (entry->value == NULL)
    {
      free (entry->key);
      free (entry);
      g_last_error = MAP_NO_MEMORY;
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

static map_entry_t *
resize_map (map_t *map)
{
  g_last_error = MAP_OK;
  size_t new_capacity = map->bucket_count * 2;
  map_entry_t **new_buckets
      = (map_entry_t **)calloc (new_capacity, sizeof (map_entry_t *));
  if (new_buckets == NULL)
    {
      g_last_error = MAP_NO_MEMORY;
      return NULL;
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

  return *map->buckets;
}

map_t *
map_insert (map_t *map, const void *key, const void *value)
{
  g_last_error = MAP_OK;
  if (map == NULL || key == NULL || value == NULL)
    {
      g_last_error = MAP_NULL_PTR;
      return NULL;
    }

  float load_factor = (float)map->size / (float)map->bucket_count;
  if (load_factor >= 0.75f)
    {
      if (resize_map (map) == NULL)
        {
          return NULL; // error already set by resize_map
        }
    }

  size_t index = map->hash_fn (key) % map->bucket_count;
  map_entry_t *entry = map->buckets[index];

  while (entry != NULL)
    {
      if (map->eq_fn (entry->key, key))
        {
          g_last_error = MAP_DUPLICATE_KEY;
          return NULL;
        }
      entry = entry->next;
    }

  map_entry_t *new_entry = create_entry (map, key, value);
  if (new_entry == NULL)
    {
      return NULL; // error already set by create_entry
    }

  new_entry->next = map->buckets[index];
  map->buckets[index] = new_entry;
  map->size++;

  return map;
}

void *
map_get (const map_t *map, const void *key)
{
  g_last_error = MAP_OK;

  if (map == NULL || key == NULL)
    {
      g_last_error = MAP_NULL_PTR;
      return NULL;
    }

  size_t index = map->hash_fn (key) % map->bucket_count;
  map_entry_t *entry = map->buckets[index];

  while (entry != NULL)
    {
      if (map->eq_fn (entry->key, key))
        {
          void *result = malloc (map->value_size);
          if (result == NULL)
            {
              g_last_error = MAP_NO_MEMORY;
              return NULL;
            }
          memcpy (result, entry->value, map->value_size);
          g_last_error = MAP_OK;
          return result;
        }
      entry = entry->next;
    }

  g_last_error = MAP_KEY_NOT_FOUND;
  return NULL;
}

map_t *
map_remove (map_t *map, const void *key)
{
  g_last_error = MAP_OK;

  if (map == NULL || key == NULL)
    {
      g_last_error = MAP_NULL_PTR;
      return NULL;
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
          g_last_error = MAP_OK;
          return map;
        }
      prev = entry;
      entry = entry->next;
    }

  g_last_error = MAP_KEY_NOT_FOUND;
  return NULL;
}

size_t
map_size (const map_t *map)
{
  g_last_error = MAP_OK;

  if (map == NULL)
    {
      g_last_error = MAP_NULL_PTR;
      return 0;
    }
  return map->size;
}

bool
map_contains (const map_t *map, const void *key)
{
  g_last_error = MAP_OK;

  if (map == NULL || key == NULL)
    {
      g_last_error = MAP_NULL_PTR;
      return false;
    }

  size_t index = map->hash_fn (key) % map->bucket_count;
  map_entry_t *entry = map->buckets[index];

  while (entry != NULL)
    {
      if (map->eq_fn (entry->key, key))
        {
          g_last_error = MAP_OK;
          return true;
        }
      entry = entry->next;
    }

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
