/*
 * sort_internal.h - helpers shared by the algorithm implementations.
 * Not part of the public interface.
 */
#ifndef SORT_INTERNAL_H
#define SORT_INTERNAL_H

#include <stddef.h>

#include "sort.h"

/* Returns SORT_OK for (NULL, 0) and valid pointers, SORT_INVALID_ARGUMENT otherwise. */
SortStatus sort_check_arguments(const int *array, size_t length);

/*
 * Stable insertion sort over array[low, high).
 * Requires high <= the array length and low <= high.
 */
void sort_insertion_range(int *array, size_t low, size_t high);

/* Heap sort over array[low, high); used by quicksort's introsort fallback. */
void sort_heap_range(int *array, size_t low, size_t high);

static inline void sort_swap_int(int *lhs, int *rhs)
{
    int temporary = *lhs;
    *lhs = *rhs;
    *rhs = temporary;
}

#endif /* SORT_INTERNAL_H */
