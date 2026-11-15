/*
 * sort_counting.c - stable counting sort for int values.
 *
 * Needs one counter per value in [min, max]. Spans above
 * SORT_COUNTING_MAX_RANGE are rejected with SORT_RANGE_ERROR instead of
 * attempting an unbounded allocation. Stable, not in place.
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "sort_internal.h"

SortStatus sort_counting(int *array, size_t length)
{
    SortStatus status = sort_check_arguments(array, length);
    if (status != SORT_OK) {
        return status;
    }
    if (length < 2) {
        return SORT_OK;
    }
    if (length > SIZE_MAX / sizeof *array) {
        return SORT_OUT_OF_MEMORY;
    }

    int minimum = array[0];
    int maximum = array[0];
    for (size_t index = 1; index < length; ++index) {
        if (array[index] < minimum) {
            minimum = array[index];
        }
        if (array[index] > maximum) {
            maximum = array[index];
        }
    }

    /* Use unsigned arithmetic so INT_MAX - INT_MIN cannot overflow. */
    unsigned long long span =
        (unsigned long long)((long long)maximum - (long long)minimum) + 1ULL;
    if (span > (unsigned long long)SORT_COUNTING_MAX_RANGE) {
        return SORT_RANGE_ERROR;
    }
    size_t range = (size_t)span;

    size_t *counts = (size_t *)calloc(range, sizeof *counts);
    int *scratch = (int *)malloc(length * sizeof *scratch);
    if (counts == NULL || scratch == NULL) {
        free(counts);
        free(scratch);
        return SORT_OUT_OF_MEMORY;
    }

    for (size_t index = 0; index < length; ++index) {
        size_t slot = (size_t)((long long)array[index] - (long long)minimum);
        ++counts[slot];
    }

    /* Counts become start offsets; scattering forwards keeps equal values stable. */
    size_t offset = 0;
    for (size_t slot = 0; slot < range; ++slot) {
        size_t count = counts[slot];
        counts[slot] = offset;
        offset += count;
    }
    for (size_t index = 0; index < length; ++index) {
        size_t slot = (size_t)((long long)array[index] - (long long)minimum);
        scratch[counts[slot]++] = array[index];
    }

    memcpy(array, scratch, length * sizeof *array);
    free(scratch);
    free(counts);
    return SORT_OK;
}
