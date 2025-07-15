#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Default initial capacity for new vectors
 */
#define DEFAULT_CAPACITY 8

/**
 * Growth factor for vector expansion
 */
#define GROWTH_FACTOR 2

Vector *vector_create(size_t initial_capacity) {
  if (initial_capacity == 0) {
    initial_capacity = DEFAULT_CAPACITY;
  }

  Vector *vec = malloc(sizeof(Vector));
  if (!vec) {
    return NULL;
  }

  vec->data = malloc(initial_capacity * sizeof(int));
  if (!vec->data) {
    free(vec);
    return NULL;
  }

  vec->size = 0;
  vec->capacity = initial_capacity;

  return vec;
}

void vector_destroy(Vector *vec) {
  if (vec) {
    free(vec->data);
    free(vec);
  }
}

/**
 * Internal function to resize the vector capacity
 */
static int vector_resize(Vector *vec, size_t new_capacity) {
  int *new_data = realloc(vec->data, new_capacity * sizeof(int));
  if (!new_data) {
    return -1;
  }

  vec->data = new_data;
  vec->capacity = new_capacity;
  return 0;
}

int vector_push(Vector *vec, int value) {
  if (!vec) {
    return -1;
  }

  // Resize if necessary
  if (vec->size >= vec->capacity) {
    size_t new_capacity = vec->capacity * GROWTH_FACTOR;
    if (vector_resize(vec, new_capacity) != 0) {
      return -1;
    }
  }

  vec->data[vec->size] = value;
  vec->size++;

  return 0;
}

int vector_pop(Vector *vec, int *value) {
  if (!vec || vec->size == 0) {
    return -1;
  }

  vec->size--;
  if (value) {
    *value = vec->data[vec->size];
  }

  // Shrink if using less than 1/4 of capacity
  if (vec->size < vec->capacity / 4 && vec->capacity > DEFAULT_CAPACITY) {
    size_t new_capacity = vec->capacity / GROWTH_FACTOR;
    vector_resize(vec, new_capacity); // Ignore failure for shrinking
  }

  return 0;
}

int vector_get(const Vector *vec, size_t index, int *value) {
  if (!vec || index >= vec->size || !value) {
    return -1;
  }

  *value = vec->data[index];
  return 0;
}

int vector_set(Vector *vec, size_t index, int value) {
  if (!vec || index >= vec->size) {
    return -1;
  }

  vec->data[index] = value;
  return 0;
}

size_t vector_size(const Vector *vec) { return vec ? vec->size : 0; }

void vector_print(const Vector *vec) {
  if (!vec) {
    printf("Vector is NULL\n");
    return;
  }

  printf("Vector[%zu/%zu]: [", vec->size, vec->capacity);
  for (size_t i = 0; i < vec->size; i++) {
    printf("%d", vec->data[i]);
    if (i < vec->size - 1) {
      printf(", ");
    }
  }
  printf("]\n");
}