/*
 * sort_support.c - argument validation, status names and shared helpers.
 */
#include "sort_internal.h"

SortStatus sort_check_arguments(const int *array, size_t length)
{
    if (length > 0 && array == NULL) {
        return SORT_INVALID_ARGUMENT;
    }
    return SORT_OK;
}

const char *sort_status_string(SortStatus status)
{
    switch (status) {
    case SORT_OK:
        return "SORT_OK";
    case SORT_INVALID_ARGUMENT:
        return "SORT_INVALID_ARGUMENT";
    case SORT_OUT_OF_MEMORY:
        return "SORT_OUT_OF_MEMORY";
    case SORT_RANGE_ERROR:
        return "SORT_RANGE_ERROR";
    }
    return "unknown SortStatus";
}

void sort_insertion_range(int *array, size_t low, size_t high)
{
    for (size_t index = low; index + 1 < high; ++index) {
        int value = array[index + 1];
        size_t position = index + 1;
        while (position > low && array[position - 1] > value) {
            array[position] = array[position - 1];
            --position;
        }
        array[position] = value;
    }
}
