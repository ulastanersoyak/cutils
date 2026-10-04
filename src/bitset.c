#include "cutils/bitset.h"
#include "cutils/config.h"
#include "internal/capacity.h"

#include <stdint.h>
#include <string.h>
#include <threads.h>

#define WORD_BITS 32u

static thread_local bitset_result_t g_last_error = BITSET_OK;

static size_t
word_index (size_t index)
{
  return index / WORD_BITS;
}

static uint32_t
bit_mask (size_t index)
{
  return (uint32_t)1u << (index % WORD_BITS);
}

// bits of the last word that are part of the set. the rest must stay clear
// so counting and searching don't see them
static uint32_t
last_word_mask (const bitset_t *bitset)
{
  size_t used = bitset->bits % WORD_BITS;
  return used == 0 ? UINT32_MAX : ((uint32_t)1u << used) - 1u;
}

static size_t
lowest_set_bit (uint32_t word)
{
  size_t position = 0;
  while ((word & 1u) == 0)
    {
      word >>= 1;
      position++;
    }
  return position;
}

static size_t
count_bits (uint32_t word)
{
  size_t count = 0;
  while (word != 0)
    {
      word &= word - 1u;
      count++;
    }
  return count;
}

// common checks for the single bit operations
static bool
valid_index (const bitset_t *bitset, size_t index)
{
  if (bitset == NULL)
    {
      g_last_error = BITSET_NULL_PTR;
      return false;
    }
  if (index >= bitset->bits)
    {
      g_last_error = BITSET_OUT_OF_RANGE;
      return false;
    }
  return true;
}

bitset_t *
bitset_create_with_allocator (size_t bits, cutils_allocator_t *allocator)
{
  g_last_error = BITSET_OK;

  if (bits == 0 || allocator == NULL)
    {
      g_last_error = BITSET_INVALID_ARG;
      return NULL;
    }

  size_t word_count = bits / WORD_BITS + (bits % WORD_BITS != 0);
  size_t bytes;
  if (!cutils_array_bytes (word_count, sizeof (uint32_t), &bytes))
    {
      g_last_error = BITSET_INVALID_ARG;
      return NULL;
    }

  bitset_t *bitset = cutils_allocate_aligned (allocator, sizeof (bitset_t),
                                              CUTILS_ALIGNMENT);
  if (bitset == NULL)
    {
      g_last_error = BITSET_NO_MEMORY;
      return NULL;
    }

  bitset->words = cutils_allocate_aligned (allocator, bytes, CUTILS_ALIGNMENT);
  if (bitset->words == NULL)
    {
      cutils_deallocate (allocator, bitset);
      g_last_error = BITSET_NO_MEMORY;
      return NULL;
    }

  memset (bitset->words, 0, bytes);
  bitset->bits = bits;
  bitset->word_count = word_count;
  bitset->allocator = allocator;
  return bitset;
}

bitset_t *
bitset_create (size_t bits)
{
  return bitset_create_with_allocator (bits, cutils_default_allocator ());
}

void
bitset_destroy (bitset_t *bitset)
{
  g_last_error = BITSET_OK;

  if (bitset == NULL)
    {
      g_last_error = BITSET_NULL_PTR;
      return;
    }

  cutils_deallocate (bitset->allocator, bitset->words);
  cutils_deallocate (bitset->allocator, bitset);
}

bool
bitset_set (bitset_t *bitset, size_t index)
{
  g_last_error = BITSET_OK;
  if (!valid_index (bitset, index))
    {
      return false;
    }
  bitset->words[word_index (index)] |= bit_mask (index);
  return true;
}

bool
bitset_clear (bitset_t *bitset, size_t index)
{
  g_last_error = BITSET_OK;
  if (!valid_index (bitset, index))
    {
      return false;
    }
  bitset->words[word_index (index)] &= ~bit_mask (index);
  return true;
}

bool
bitset_toggle (bitset_t *bitset, size_t index)
{
  g_last_error = BITSET_OK;
  if (!valid_index (bitset, index))
    {
      return false;
    }
  bitset->words[word_index (index)] ^= bit_mask (index);
  return true;
}

bool
bitset_test (const bitset_t *bitset, size_t index)
{
  g_last_error = BITSET_OK;
  if (!valid_index (bitset, index))
    {
      return false;
    }
  return (bitset->words[word_index (index)] & bit_mask (index)) != 0;
}

bool
bitset_set_all (bitset_t *bitset)
{
  g_last_error = BITSET_OK;

  if (bitset == NULL)
    {
      g_last_error = BITSET_NULL_PTR;
      return false;
    }

  memset (bitset->words, 0xFF, bitset->word_count * sizeof (uint32_t));
  bitset->words[bitset->word_count - 1] &= last_word_mask (bitset);
  return true;
}

bool
bitset_clear_all (bitset_t *bitset)
{
  g_last_error = BITSET_OK;

  if (bitset == NULL)
    {
      g_last_error = BITSET_NULL_PTR;
      return false;
    }

  memset (bitset->words, 0, bitset->word_count * sizeof (uint32_t));
  return true;
}

size_t
bitset_count (const bitset_t *bitset)
{
  g_last_error = BITSET_OK;

  if (bitset == NULL)
    {
      g_last_error = BITSET_NULL_PTR;
      return 0;
    }

  size_t count = 0;
  for (size_t i = 0; i < bitset->word_count; i++)
    {
      count += count_bits (bitset->words[i]);
    }
  return count;
}

size_t
bitset_find_first_set (const bitset_t *bitset)
{
  g_last_error = BITSET_OK;

  if (bitset == NULL)
    {
      g_last_error = BITSET_NULL_PTR;
      return SIZE_MAX;
    }

  for (size_t i = 0; i < bitset->word_count; i++)
    {
      if (bitset->words[i] != 0)
        {
          return i * WORD_BITS + lowest_set_bit (bitset->words[i]);
        }
    }
  return SIZE_MAX;
}

size_t
bitset_find_first_clear (const bitset_t *bitset)
{
  g_last_error = BITSET_OK;

  if (bitset == NULL)
    {
      g_last_error = BITSET_NULL_PTR;
      return SIZE_MAX;
    }

  for (size_t i = 0; i < bitset->word_count; i++)
    {
      uint32_t valid
          = i + 1 == bitset->word_count ? last_word_mask (bitset) : UINT32_MAX;
      uint32_t clear = ~bitset->words[i] & valid;
      if (clear != 0)
        {
          return i * WORD_BITS + lowest_set_bit (clear);
        }
    }
  return SIZE_MAX;
}

size_t
bitset_size (const bitset_t *bitset)
{
  if (bitset == NULL)
    {
      g_last_error = BITSET_NULL_PTR;
      return 0;
    }
  return bitset->bits;
}

bitset_result_t
bitset_get_error (void)
{
  return g_last_error;
}
