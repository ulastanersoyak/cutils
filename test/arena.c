#include "cutils/arena.h"
#include "test.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_arena_create (void);
static void test_arena_alloc (void);
static void test_arena_alloc_aligned (void);
static void test_arena_reset (void);
static void test_arena_total_size (void);
static void test_arena_used_size (void);
static void test_arena_block_count (void);

void
run_arena_tests (void)
{
  test_arena_create ();
  test_arena_alloc ();
  test_arena_alloc_aligned ();
  test_arena_reset ();
  test_arena_total_size ();
  test_arena_used_size ();
  test_arena_block_count ();
}

static void
test_arena_create (void)
{
  // Test normal creation
  arena_t *arena = arena_create (1024);
  assert (arena != NULL);
  assert (arena->block_size == 1024);
  assert (arena->first != NULL);
  assert (arena->current != NULL);
  assert (arena->first == arena->current);
  arena_destroy (arena);

  // Test zero block size
  arena = arena_create (0);
  assert (arena == NULL);
  assert (arena_get_error () == ARENA_INVALID_ARG);

  TEST_PASS ("arena_create");
}

static void
test_arena_alloc (void)
{
  arena_t *arena = arena_create (64);

  // Test basic allocation
  int32_t *num = arena_alloc (arena, sizeof (int32_t));
  assert (num != NULL);
  *num = 42;
  assert (*num == 42);

  // Test allocation that spans multiple blocks
  char *large = arena_alloc (arena, 100); // Should create new block
  assert (large != NULL);
  memset (large, 'A', 100); // Should not crash

  // Test null arena
  void *ptr = arena_alloc (NULL, sizeof (int32_t));
  assert (ptr == NULL);
  assert (arena_get_error () == ARENA_NULL_PTR);

  // Test zero size
  ptr = arena_alloc (arena, 0);
  assert (ptr == NULL);
  assert (arena_get_error () == ARENA_INVALID_ARG);

  arena_destroy (arena);
  TEST_PASS ("arena_alloc");
}

static void
test_arena_alloc_aligned (void)
{
  arena_t *arena = arena_create (64);

  // Test with different alignments
  void *ptr1 = arena_alloc_aligned (arena, 8, 8);
  assert (ptr1 != NULL);
  assert (((uintptr_t)ptr1 & 7) == 0); // Should be 8-byte aligned

  void *ptr2 = arena_alloc_aligned (arena, 8, 16);
  assert (ptr2 != NULL);
  assert (((uintptr_t)ptr2 & 15) == 0); // Should be 16-byte aligned

  // Test invalid alignment (not power of 2)
  void *ptr3 = arena_alloc_aligned (arena, 8, 3);
  assert (ptr3 == NULL);
  assert (arena_get_error () == ARENA_INVALID_ARG);

  arena_destroy (arena);
  TEST_PASS ("arena_alloc_aligned");
}

static void
test_arena_reset (void)
{
  arena_t *arena = arena_create (64);

  // Allocate some memory
  for (int i = 0; i < 10; i++)
    {
      int32_t *num = arena_alloc (arena, sizeof (int32_t));
      assert (num != NULL);
      *num = i;
    }

  // Test reset
  assert (arena_reset (arena) == true);
  assert (arena->current == arena->first);
  assert (arena->first->used == 0);

  // Test null arena
  assert (arena_reset (NULL) == false);
  assert (arena_get_error () == ARENA_NULL_PTR);

  arena_destroy (arena);
  TEST_PASS ("arena_reset");
}

static void
test_arena_total_size (void)
{
  arena_t *arena = arena_create (64);

  // Test initial size
  assert (arena_total_size (arena) == 64);

  // Force allocation of new blocks
  void *ptr = arena_alloc (arena, 100); // Should create new block
  assert (ptr != NULL);
  assert (arena_total_size (arena) >= 128); // At least two blocks

  // Test null arena
  assert (arena_total_size (NULL) == 0);
  assert (arena_get_error () == ARENA_NULL_PTR);

  arena_destroy (arena);
  TEST_PASS ("arena_total_size");
}

static void
test_arena_used_size (void)
{
  arena_t *arena = arena_create (64);

  // Test initial used size
  assert (arena_used_size (arena) == 0);

  // Test after allocations
  int32_t *num = arena_alloc (arena, sizeof (int32_t));
  assert (num != NULL);
  assert (arena_used_size (arena) == sizeof (int32_t));

  // Test null arena
  assert (arena_used_size (NULL) == 0);
  assert (arena_get_error () == ARENA_NULL_PTR);

  arena_destroy (arena);
  TEST_PASS ("arena_used_size");
}

static void
test_arena_block_count (void)
{
  arena_t *arena = arena_create (64);

  // Test initial block count
  assert (arena_block_count (arena) == 1);

  // Force allocation of new blocks
  void *ptr = arena_alloc (arena, 100); // Should create new block
  assert (ptr != NULL);
  assert (arena_block_count (arena) == 2);

  // Test null arena
  assert (arena_block_count (NULL) == 0);
  assert (arena_get_error () == ARENA_NULL_PTR);

  arena_destroy (arena);
  TEST_PASS ("arena_block_count");
}
