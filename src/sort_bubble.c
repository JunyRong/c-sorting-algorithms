/*
 * sort_bubble.c - bubble sort with early exit. Stable, in place.
 */
#include "sort_internal.h"

SortStatus sort_bubble(int *array, size_t length)
{
    SortStatus status = sort_check_arguments(array, length);
    if (status != SORT_OK) {
        return status;
    }

    /* Each pass moves the largest remaining value to the end of the range. */
    for (size_t pass = 0; pass + 1 < length; ++pass) {
        size_t last = length - 1 - pass;
        int swapped = 0;
        for (size_t index = 0; index < last; ++index) {
            if (array[index] > array[index + 1]) {
                sort_swap_int(&array[index], &array[index + 1]);
                swapped = 1;
            }
        }
        if (!swapped) {
            break;
        }
    }
    return SORT_OK;
}
