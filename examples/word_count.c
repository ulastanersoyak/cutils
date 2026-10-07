// counts the words read from stdin and prints the most frequent ones:
//   ./word_count < some_text.txt
//
// shows a hashmap with custom hash and equality functions for string keys,
// string_t for building each word, and a map for sorting the results.
#include <ctype.h>
#include <cutils/cutils.h>
#include <stdio.h>
#include <string.h>

#define MAX_WORD 32
#define TOP 10

typedef struct
{
  char text[MAX_WORD];
} word_t;

static uint32_t
hash_word (const void *key)
{
  const char *s = ((const word_t *)key)->text;
  uint32_t hash = 2166136261u;
  while (*s != '\0')
    {
      hash = (hash ^ (unsigned char)*s++) * 16777619u;
    }
  return hash;
}

static bool
equal_words (const void *a, const void *b)
{
  return strcmp (((const word_t *)a)->text, ((const word_t *)b)->text) == 0;
}

// orders by count, highest first, then alphabetically
typedef struct
{
  int count;
  word_t word;
} ranked_t;

static int
compare_ranked (const void *a, const void *b)
{
  const ranked_t *x = a;
  const ranked_t *y = b;
  if (x->count != y->count)
    {
      return x->count > y->count ? -1 : 1;
    }
  return strcmp (x->word.text, y->word.text);
}

static void
count_word (hashmap_t *counts, string_t *current)
{
  if (string_is_empty (current))
    {
      return;
    }

  word_t word = { { 0 } };
  strncpy (word.text, string_cstr (current), MAX_WORD - 1);
  int *count = hashmap_get_ptr (counts, &word);
  if (count != NULL)
    {
      (*count)++;
    }
  else
    {
      int one = 1;
      hashmap_insert (counts, &word, &one);
    }
  string_clear (current);
}

int
main (void)
{
  hashmap_t *counts
      = hashmap_create (sizeof (word_t), sizeof (int), hash_word, equal_words);
  string_t *current = string_create (MAX_WORD);
  if (counts == NULL || current == NULL)
    {
      fprintf (stderr, "out of memory\n");
      return 1;
    }

  int c;
  while ((c = getchar ()) != EOF)
    {
      if (isalpha (c) || c == '\'')
        {
          string_append_char (current, (char)tolower (c));
        }
      else
        {
          count_word (counts, current);
        }
    }
  count_word (counts, current);

  // a map keyed by (count, word) keeps the results sorted as they go in
  map_t *ranking
      = map_create (sizeof (ranked_t), sizeof (char), compare_ranked);
  size_t cursor = 0;
  ranked_t entry = { 0 };
  char unused = 0;
  while (hashmap_next (counts, &cursor, &entry.word, &entry.count))
    {
      map_insert (ranking, &entry, &unused);
    }

  printf ("%zu different words\n", hashmap_size (counts));
  int shown = 0;
  for (map_iterator_t it = map_begin (ranking);
       shown < TOP && map_iterator_get (&it, &entry, &unused);
       map_iterator_next (&it), shown++)
    {
      printf ("%6d  %s\n", entry.count, entry.word.text);
    }

  map_destroy (ranking);
  string_destroy (current);
  hashmap_destroy (counts);
  return 0;
}
