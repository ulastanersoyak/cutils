#include "test.h"

#include <stdio.h>
#include <stdlib.h>

// Global test statistics
test_stats_t test_stats = {0, 0, 0};

void run_all_tests(void) {
    printf("\n=== Starting CUtils Test Suite ===\n\n");

    // Run all test suites
    printf("Running allocator tests...\n");
    run_allocator_tests();
    printf("\n");

    printf("Running arena tests...\n");
    run_arena_tests();
    printf("\n");

    printf("Running expected tests...\n");
    run_expected_tests();
    printf("\n");

    printf("Running list tests...\n");
    run_list_tests();
    printf("\n");

    printf("Running map tests...\n");
    run_map_tests();
    printf("\n");

    printf("Running priority queue tests...\n");
    run_priority_queue_tests();
    printf("\n");

    printf("Running queue tests...\n");
    run_queue_tests();
    printf("\n");

    printf("Running stack tests...\n");
    run_stack_tests();
    printf("\n");

    printf("Running string tests...\n");
    run_string_tests();
    printf("\n");

    printf("Running vector tests...\n");
    run_vector_tests();
    printf("\n");

    // Print final statistics
    printf("=== Test Results ===\n");
    printf("Total tests: %d\n", test_stats.total);
    printf("Passed: %d\n", test_stats.passed);
    printf("Failed: %d\n", test_stats.failed);
    printf("Success rate: %.1f%%\n", 
           (test_stats.total > 0) ? 
           ((float)test_stats.passed / test_stats.total * 100.0f) : 0.0f);

    // Exit with appropriate status code
    exit(test_stats.failed > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}

int main(void) {
    run_all_tests();
    return 0;
}
