#include "test.h"

int
main (void)
{

  printf ("\nRunning all test suites...\n\n");

  printf ("String Tests:\n");
  printf ("-----------------\n");
  run_string_tests ();
  printf ("-----------------\n");
  printf ("Vector Tests:\n");
  printf ("-----------------\n");
  run_vector_tests ();
  printf ("-----------------\n");
  printf ("File Tests:\n");
  printf ("-----------------\n");
  run_file_tests ();
  printf ("-----------------\n");
  printf ("Sort Tests:\n");
  printf ("-----------------\n");
  run_sort_tests ();
  printf ("-----------------\n");

  printf ("\nAll tests completed!\n");
  return 0;
}
