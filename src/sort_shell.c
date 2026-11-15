/*
 * sort_shell.c - Shell sort with Knuth's 3k + 1 gap sequence.
 *
 * Worst case O(n^1.5) for this gap sequence; not stable, in place.
 */
#include "sort_internal.h"

SortStatus sort_shell(int *array, size_t length)
{
    SortStatus status = sort_check_arguments(array, length);
    if (status != SORT_OK) {
        return status;
    }

    size_t gap = 1;
    while (gap < length / 3) {
        gap = gap * 3 + 1;
    }

    for (; gap > 0; gap /= 3) {
        for (size_t index = gap; index < length; ++index) {
            int value = array[index];
            size_t position = index;
            while (position >= gap && array[position - gap] > value) {
                array[position] = array[position - gap];
                position -= gap;
            }
            array[position] = value;
        }
    }
    return SORT_OK;
}
