#ifndef CUTILS_TEST_H
#define CUTILS_TEST_H

#include <stdbool.h>
#include <stdio.h>

#define TEST_PASS(name) printf ("✓ %s\n", name)
#define TEST_FAIL(name, msg) printf ("✗ %s: %s\n", name, msg)

typedef void (*test_suite_fn) (void);

void run_string_tests (void);
void run_vector_tests (void);

#endif
