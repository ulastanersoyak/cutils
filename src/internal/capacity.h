#ifndef CUTILS_CAPACITY_H
#define CUTILS_CAPACITY_H

/* Internal helpers shared by the array based containers. Not installed. */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Capacity used when a container created with capacity 0 first grows */
#define CUTILS_MIN_CAPACITY 4

/* Stores count * elem_size in *bytes. Returns false if it would overflow */
static inline bool
cutils_array_bytes (size_t count, size_t elem_size, size_t *bytes)
{
  if (elem_size != 0 && count > SIZE_MAX / elem_size)
    {
      return false;
    }
  *bytes = count * elem_size;
  return true;
}

/* Doubles current until it reaches required, starting from
   CUTILS_MIN_CAPACITY when current is 0. Returns false if the result
   would exceed max */
static inline bool
cutils_next_capacity (size_t current, size_t required, size_t max,
                      size_t *next)
{
  size_t capacity = current != 0 ? current : CUTILS_MIN_CAPACITY;
  while (capacity < required)
    {
      if (capacity > SIZE_MAX / 2)
        {
          return false;
        }
      capacity *= 2;
    }
  if (capacity > max)
    {
      // doubling overshot, but the request itself may still fit
      if (required > max)
        {
          return false;
        }
      capacity = max;
    }
  *next = capacity;
  return true;
}

#endif /* CUTILS_CAPACITY_H */
