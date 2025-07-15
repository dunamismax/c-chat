/**
 * @file vector.h
 * @brief Dynamic array implementation optimized for ARM64
 * @author dunamismax <dunamismax@tutamail.com>
 * @version 1.0.0
 * @date 2024
 *
 * High-performance dynamic array (vector) implementation in C
 * Optimized for Apple Silicon (ARM64) architecture
 */

#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>

/**
 * @brief Dynamic array implementation in C
 *
 * Demonstrates modern C memory management and data structures.
 * Features automatic resizing, bounds checking, and ARM64 optimizations.
 */
typedef struct {
  int *data;       // Pointer to the array data
  size_t size;     // Current number of elements
  size_t capacity; // Maximum capacity before reallocation
} Vector;

/**
 * Create a new vector with initial capacity
 * @param initial_capacity Initial capacity of the vector
 * @return Pointer to new vector or NULL on failure
 */
Vector *vector_create(size_t initial_capacity);

/**
 * Destroy a vector and free all memory
 * @param vec Vector to destroy
 */
void vector_destroy(Vector *vec);

/**
 * Add an element to the end of the vector
 * @param vec Vector to append to
 * @param value Value to append
 * @return 0 on success, -1 on failure
 */
int vector_push(Vector *vec, int value);

/**
 * Remove and return the last element
 * @param vec Vector to pop from
 * @param value Pointer to store the popped value
 * @return 0 on success, -1 if vector is empty
 */
int vector_pop(Vector *vec, int *value);

/**
 * Get element at specified index
 * @param vec Vector to access
 * @param index Index of element
 * @param value Pointer to store the value
 * @return 0 on success, -1 on invalid index
 */
int vector_get(const Vector *vec, size_t index, int *value);

/**
 * Set element at specified index
 * @param vec Vector to modify
 * @param index Index of element
 * @param value New value
 * @return 0 on success, -1 on invalid index
 */
int vector_set(Vector *vec, size_t index, int value);

/**
 * Get the current size of the vector
 * @param vec Vector to query
 * @return Current size
 */
size_t vector_size(const Vector *vec);

/**
 * Print all elements in the vector
 * @param vec Vector to print
 */
void vector_print(const Vector *vec);

#endif /* VECTOR_H */
