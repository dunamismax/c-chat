#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

// Include the test framework
#include "../test_framework.h"

// Include the vector library
#include "../../libs/data_structures/include/vector.h"

// Test vector creation and destruction
int test_create_destroy(void) {
  Vector *vec = vector_create(5);
  if (!vec)
    return 0;

  if (vector_size(vec) != 0) {
    vector_destroy(vec);
    return 0;
  }

  vector_destroy(vec);
  return 1;
}

// Test push and size operations
int test_push_and_size(void) {
  Vector *vec = vector_create(2);
  if (!vec)
    return 0;

  // Test pushing elements
  if (vector_push(vec, 10) != 0) {
    vector_destroy(vec);
    return 0;
  }

  if (vector_size(vec) != 1) {
    vector_destroy(vec);
    return 0;
  }

  if (vector_push(vec, 20) != 0) {
    vector_destroy(vec);
    return 0;
  }

  if (vector_size(vec) != 2) {
    vector_destroy(vec);
    return 0;
  }

  // Test growth beyond initial capacity
  if (vector_push(vec, 30) != 0) {
    vector_destroy(vec);
    return 0;
  }

  if (vector_size(vec) != 3) {
    vector_destroy(vec);
    return 0;
  }

  vector_destroy(vec);
  return 1;
}

// Test get and set operations
int test_get_set(void) {
  Vector *vec = vector_create(3);
  if (!vec)
    return 0;

  // Add some elements
  vector_push(vec, 100);
  vector_push(vec, 200);
  vector_push(vec, 300);

  // Test get
  int value;
  if (vector_get(vec, 0, &value) != 0 || value != 100) {
    vector_destroy(vec);
    return 0;
  }

  if (vector_get(vec, 1, &value) != 0 || value != 200) {
    vector_destroy(vec);
    return 0;
  }

  if (vector_get(vec, 2, &value) != 0 || value != 300) {
    vector_destroy(vec);
    return 0;
  }

  // Test set
  if (vector_set(vec, 1, 250) != 0) {
    vector_destroy(vec);
    return 0;
  }

  if (vector_get(vec, 1, &value) != 0 || value != 250) {
    vector_destroy(vec);
    return 0;
  }

  // Test invalid indices
  if (vector_get(vec, 5, &value) == 0) { // Should fail
    vector_destroy(vec);
    return 0;
  }

  if (vector_set(vec, 5, 500) == 0) { // Should fail
    vector_destroy(vec);
    return 0;
  }

  vector_destroy(vec);
  return 1;
}

// Test pop operation
int test_pop(void) {
  Vector *vec = vector_create(3);
  if (!vec)
    return 0;

  // Add elements
  vector_push(vec, 10);
  vector_push(vec, 20);
  vector_push(vec, 30);

  // Test pop
  int value;
  if (vector_pop(vec, &value) != 0 || value != 30) {
    vector_destroy(vec);
    return 0;
  }

  if (vector_size(vec) != 2) {
    vector_destroy(vec);
    return 0;
  }

  if (vector_pop(vec, &value) != 0 || value != 20) {
    vector_destroy(vec);
    return 0;
  }

  if (vector_pop(vec, &value) != 0 || value != 10) {
    vector_destroy(vec);
    return 0;
  }

  if (vector_size(vec) != 0) {
    vector_destroy(vec);
    return 0;
  }

  // Test pop from empty vector
  if (vector_pop(vec, &value) == 0) { // Should fail
    vector_destroy(vec);
    return 0;
  }

  vector_destroy(vec);
  return 1;
}

// Test edge cases
int test_edge_cases(void) {
  // Test NULL vector
  if (vector_size(NULL) != 0)
    return 0;

  // Test zero capacity creation
  Vector *vec = vector_create(0);
  if (!vec)
    return 0;

  // Should still be able to push (will use default capacity)
  if (vector_push(vec, 42) != 0) {
    vector_destroy(vec);
    return 0;
  }

  vector_destroy(vec);
  return 1;
}

int main(void) {
  printf("Vector Library Test Suite\n");
  printf("========================\n\n");

  run_test(test_create_destroy, "test_create_destroy");
  run_test(test_push_and_size, "test_push_and_size");
  run_test(test_get_set, "test_get_set");
  run_test(test_pop, "test_pop");
  run_test(test_edge_cases, "test_edge_cases");

  return test_summary("Vector Library Test Suite");
}
