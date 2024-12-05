#include "test.h"

int
main (void)
{

  printf ("\nRunning all test suites...\n\n");

  printf ("String Tests:\n");
  printf ("-----------------\n");
  run_string_tests ();

  printf ("\nAll tests completed!\n");
  return 0;
}
