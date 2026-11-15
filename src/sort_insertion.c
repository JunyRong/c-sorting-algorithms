/*
 * sort_insertion.c - insertion sort. Stable, in place, adaptive.
 */
#include "sort_internal.h"

SortStatus sort_insertion(int *array, size_t length)
{
    SortStatus status = sort_check_arguments(array, length);
    if (status != SORT_OK) {
        return status;
    }

    sort_insertion_range(array, 0, length);
    return SORT_OK;
}
