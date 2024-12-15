#include "test.h"
#include <stdlib.h>

int
main (void)
{
  printf ("\nrunning all test suites\n\n");

  printf ("-----------------\n");
  printf ("string tests:\n");
  printf ("-----------------\n");
  run_string_tests ();
  printf ("-----------------\n");
  printf ("vector tests:\n");
  printf ("-----------------\n");
  run_vector_tests ();
  printf ("-----------------\n");
  printf ("file tests:\n");
  printf ("-----------------\n");
  run_file_tests ();
  printf ("-----------------\n");
  printf ("sort tests:\n");
  printf ("-----------------\n");
  run_sort_tests ();
  printf ("-----------------\n");
  printf ("sort tests:\n");
  printf ("-----------------\n");
  run_expected_tests ();
  printf ("-----------------\n");
  printf ("map tests:\n");
  printf ("-----------------\n");
  run_map_tests ();
  printf ("-----------------\n");
  printf ("stack tests:\n");
  printf ("-----------------\n");
  run_stack_tests ();
  printf ("-----------------\n");
  printf ("arena tests:\n");
  printf ("-----------------\n");
  run_arena_tests ();
  printf ("-----------------\n");

  printf ("\nall tests completed!\n");
  return EXIT_SUCCESS;
}
