/*
 * sort_cocktail.c - bidirectional bubble sort (cocktail shaker). Stable, in place.
 */
#include "sort_internal.h"

SortStatus sort_cocktail(int *array, size_t length)
{
    SortStatus status = sort_check_arguments(array, length);
    if (status != SORT_OK) {
        return status;
    }

    size_t low = 0;
    size_t high = length;
    int swapped = 1;

    while (swapped && high - low > 1) {
        swapped = 0;

        /* Forward pass: largest remaining value ends up at high - 1. */
        for (size_t index = low; index + 1 < high; ++index) {
            if (array[index] > array[index + 1]) {
                sort_swap_int(&array[index], &array[index + 1]);
                swapped = 1;
            }
        }
        --high;

        if (!swapped) {
            break;
        }

        swapped = 0;

        /* Backward pass: smallest remaining value ends up at low. */
        for (size_t index = high - 1; index > low; --index) {
            if (array[index - 1] > array[index]) {
                sort_swap_int(&array[index - 1], &array[index]);
                swapped = 1;
            }
        }
        ++low;
    }
    return SORT_OK;
}
