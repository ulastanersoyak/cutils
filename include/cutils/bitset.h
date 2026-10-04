#ifndef CUTILS_BITSET_H
#define CUTILS_BITSET_H

#include "cutils/allocator.h"
#include "cutils/config.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * Fixed size array of bits, stored in 32 bit words. Useful for flags,
 * free slot tracking and anything else that needs one bit per item.
 */
typedef struct
{
  uint32_t *words;
  size_t bits;
  size_t word_count;
  cutils_allocator_t *allocator;
} bitset_t;

typedef enum
{
  BITSET_OK = 0,
  BITSET_NULL_PTR = 1,
  BITSET_NO_MEMORY = 2,
  BITSET_INVALID_ARG = 3,
  BITSET_OUT_OF_RANGE = 4
} bitset_result_t;

/**
 * Creates a bitset with every bit cleared.
 *
 * @param bits Number of bits, must be greater than 0
 * @param allocator Allocator to use
 * @return Newly allocated bitset or NULL on error
 */
bitset_t *bitset_create_with_allocator (size_t bits,
                                        cutils_allocator_t *allocator);

/**
 * Creates a bitset with every bit cleared, using the default allocator.
 *
 * @param bits Number of bits, must be greater than 0
 * @return Newly allocated bitset or NULL on error
 */
bitset_t *bitset_create (size_t bits);

/**
 * Destroys a bitset.
 *
 * @param bitset Bitset to destroy
 */
void bitset_destroy (bitset_t *bitset);

/**
 * Sets, clears or flips one bit.
 *
 * @param bitset Bitset to change
 * @param index Bit to change
 * @return true if successful, false if bitset is NULL or index is out of
 *         range
 */
bool bitset_set (bitset_t *bitset, size_t index);
bool bitset_clear (bitset_t *bitset, size_t index);
bool bitset_toggle (bitset_t *bitset, size_t index);

/**
 * Checks one bit.
 *
 * @param bitset Bitset to read
 * @param index Bit to check
 * @return true if the bit is set, false if it is clear or on error
 */
bool bitset_test (const bitset_t *bitset, size_t index);

/**
 * Sets or clears every bit.
 *
 * @param bitset Bitset to change
 * @return true if successful, false if bitset is NULL
 */
bool bitset_set_all (bitset_t *bitset);
bool bitset_clear_all (bitset_t *bitset);

/**
 * Counts the set bits.
 *
 * @param bitset Bitset to count
 * @return Number of set bits, 0 on error
 */
size_t bitset_count (const bitset_t *bitset);

/**
 * Finds the lowest set or clear bit.
 *
 * @param bitset Bitset to search
 * @return Index of the bit, or SIZE_MAX if there is none or on error
 */
size_t bitset_find_first_set (const bitset_t *bitset);
size_t bitset_find_first_clear (const bitset_t *bitset);

/**
 * Gets the number of bits.
 *
 * @param bitset Bitset to query
 * @return Number of bits, 0 on error
 */
size_t bitset_size (const bitset_t *bitset);

/**
 * Gets the last bitset operation error.
 *
 * @return Last error code
 */
bitset_result_t bitset_get_error (void);

#endif // CUTILS_BITSET_H
