#ifndef CUTILS_TEST_H
#define CUTILS_TEST_H

#include <stdbool.h>
#include <stdio.h>

#define TEST_PASS(name) printf ("✓ %s\n", name)
#define TEST_FAIL(name, msg) printf ("✗ %s: %s\n", name, msg)

typedef void (*test_suite_fn) (void);

void run_string_tests (void);
void run_vector_tests (void);
void run_file_tests (void);
void run_sort_tests (void);
void run_expected_tests (void);
void run_map_tests (void);
void run_stack_tests (void);

#endif
