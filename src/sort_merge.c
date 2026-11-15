/*
 * sort_merge.c - bottom-up merge sort with one scratch buffer.
 * Stable, not in place, O(n) extra space.
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "sort_internal.h"

static void merge_runs(int *array, int *buffer, size_t low, size_t middle, size_t high)
{
    size_t left = low;
    size_t right = middle;
    size_t target = low;

    while (left < middle && right < high) {
        if (array[left] <= array[right]) {
            buffer[target++] = array[left++];
        } else {
            buffer[target++] = array[right++];
        }
    }
    while (left < middle) {
        buffer[target++] = array[left++];
    }
    while (right < high) {
        buffer[target++] = array[right++];
    }

    memcpy(array + low, buffer + low, (high - low) * sizeof *array);
}

SortStatus sort_merge(int *array, size_t length)
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

    int *buffer = (int *)malloc(length * sizeof *buffer);
    if (buffer == NULL) {
        return SORT_OUT_OF_MEMORY;
    }

    for (size_t width = 1; width < length;) {
        for (size_t low = 0; low < length;) {
            size_t middle = (width > length - low) ? length : low + width;
            size_t high = (width > length - middle) ? length : middle + width;

            if (middle < high && array[middle - 1] > array[middle]) {
                merge_runs(array, buffer, low, middle, high);
            }
            low = high;
        }

        /* Guard the doubling so that width can never overflow size_t. */
        if (width > length / 2) {
            break;
        }
        width *= 2;
    }

    free(buffer);
    return SORT_OK;
}
