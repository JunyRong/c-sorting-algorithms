/*
 * sort_selection.c - selection sort. Not stable, in place.
 */
#include "sort_internal.h"

SortStatus sort_selection(int *array, size_t length)
{
    SortStatus status = sort_check_arguments(array, length);
    if (status != SORT_OK) {
        return status;
    }

    for (size_t start = 0; start + 1 < length; ++start) {
        size_t smallest = start;
        for (size_t index = start + 1; index < length; ++index) {
            if (array[index] < array[smallest]) {
                smallest = index;
            }
        }
        if (smallest != start) {
            sort_swap_int(&array[start], &array[smallest]);
        }
    }
    return SORT_OK;
}
