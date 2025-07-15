/**
 * @file generic_vector.h
 * @brief Generic dynamic array implementation using void pointers
 * @author dunamismax <dunamismax@tutamail.com>
 * @version 2.0.0
 * @date 2024
 *
 * Production-ready generic vector implementation in C
 * Supports any data type through void pointers and element size specification
 */

#ifndef GENERIC_VECTOR_H
#define GENERIC_VECTOR_H

#include <stddef.h>

/**
 * @brief Generic dynamic array implementation
 *
 * Stores elements of any type using void pointers and element size.
 * Provides type safety through size validation and safe memory operations.
 */
typedef struct {
  void *data;          // Pointer to the array data
  size_t size;         // Current number of elements
  size_t capacity;     // Maximum capacity before reallocation
  size_t element_size; // Size of each element in bytes
} GenericVector;

/**
 * Comparison function type for sorting and searching
 * @param a Pointer to first element
 * @param b Pointer to second element
 * @return Negative if a < b, 0 if a == b, positive if a > b
 */
typedef int (*CompareFunc)(const void *a, const void *b);

/**
 * Element processing function for iteration
 * @param element Pointer to element
 * @param user_data User-provided data
 */
typedef void (*ProcessFunc)(void *element, void *user_data);

/**
 * Create a new generic vector with initial capacity
 * @param initial_capacity Initial capacity of the vector
 * @param element_size Size of each element in bytes
 * @return Pointer to new vector or NULL on failure
 */
GenericVector *gvector_create(size_t initial_capacity, size_t element_size);

/**
 * Destroy a vector and free all memory
 * @param vec Vector to destroy
 */
void gvector_destroy(GenericVector *vec);

/**
 * Add an element to the end of the vector
 * @param vec Vector to append to
 * @param element Pointer to element to append
 * @return 0 on success, -1 on failure
 */
int gvector_push(GenericVector *vec, const void *element);

/**
 * Remove and return the last element
 * @param vec Vector to pop from
 * @param element Pointer to store the popped element (can be NULL)
 * @return 0 on success, -1 if vector is empty
 */
int gvector_pop(GenericVector *vec, void *element);

/**
 * Get element at specified index
 * @param vec Vector to access
 * @param index Index of element
 * @param element Pointer to store the element
 * @return 0 on success, -1 on invalid index
 */
int gvector_get(const GenericVector *vec, size_t index, void *element);

/**
 * Set element at specified index
 * @param vec Vector to modify
 * @param index Index of element
 * @param element Pointer to new element value
 * @return 0 on success, -1 on invalid index
 */
int gvector_set(GenericVector *vec, size_t index, const void *element);

/**
 * Insert element at specified index
 * @param vec Vector to modify
 * @param index Index to insert at
 * @param element Pointer to element to insert
 * @return 0 on success, -1 on failure
 */
int gvector_insert(GenericVector *vec, size_t index, const void *element);

/**
 * Remove element at specified index
 * @param vec Vector to modify
 * @param index Index to remove
 * @param element Pointer to store removed element (can be NULL)
 * @return 0 on success, -1 on invalid index
 */
int gvector_remove(GenericVector *vec, size_t index, void *element);

/**
 * Get the current size of the vector
 * @param vec Vector to query
 * @return Current size
 */
size_t gvector_size(const GenericVector *vec);

/**
 * Get the current capacity of the vector
 * @param vec Vector to query
 * @return Current capacity
 */
size_t gvector_capacity(const GenericVector *vec);

/**
 * Check if vector is empty
 * @param vec Vector to check
 * @return 1 if empty, 0 if not empty, -1 on error
 */
int gvector_empty(const GenericVector *vec);

/**
 * Clear all elements from the vector
 * @param vec Vector to clear
 */
void gvector_clear(GenericVector *vec);

/**
 * Reserve capacity for the vector
 * @param vec Vector to modify
 * @param new_capacity New capacity to reserve
 * @return 0 on success, -1 on failure
 */
int gvector_reserve(GenericVector *vec, size_t new_capacity);

/**
 * Shrink vector capacity to fit current size
 * @param vec Vector to shrink
 * @return 0 on success, -1 on failure
 */
int gvector_shrink_to_fit(GenericVector *vec);

/**
 * Find first occurrence of element in vector
 * @param vec Vector to search
 * @param element Pointer to element to find
 * @param compare Comparison function
 * @return Index of element or SIZE_MAX if not found
 */
size_t gvector_find(const GenericVector *vec, const void *element,
                    CompareFunc compare);

/**
 * Sort vector elements using comparison function
 * @param vec Vector to sort
 * @param compare Comparison function
 * @return 0 on success, -1 on failure
 */
int gvector_sort(GenericVector *vec, CompareFunc compare);

/**
 * Apply function to each element in vector
 * @param vec Vector to iterate over
 * @param process Function to apply to each element
 * @param user_data User data to pass to process function
 */
void gvector_foreach(GenericVector *vec, ProcessFunc process, void *user_data);

/**
 * Create a copy of the vector
 * @param vec Vector to copy
 * @return New vector copy or NULL on failure
 */
GenericVector *gvector_copy(const GenericVector *vec);

/**
 * Get pointer to element at index (direct access)
 * WARNING: Returned pointer becomes invalid after vector operations
 * @param vec Vector to access
 * @param index Index of element
 * @return Pointer to element or NULL on invalid index
 */
void *gvector_at(const GenericVector *vec, size_t index);

/**
 * Get pointer to first element
 * @param vec Vector to access
 * @return Pointer to first element or NULL if empty
 */
void *gvector_front(const GenericVector *vec);

/**
 * Get pointer to last element
 * @param vec Vector to access
 * @return Pointer to last element or NULL if empty
 */
void *gvector_back(const GenericVector *vec);

/**
 * Get pointer to internal data array
 * WARNING: For advanced use only, direct modification can corrupt vector
 * @param vec Vector to access
 * @return Pointer to internal data array
 */
void *gvector_data(const GenericVector *vec);

#endif /* GENERIC_VECTOR_H */