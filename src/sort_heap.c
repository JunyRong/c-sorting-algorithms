/*
 * sort_heap.c - binary heap sort with a max-heap built in place.
 * Not stable, in place, O(1) extra space.
 */
#include "sort_internal.h"

static void sift_down(int *array, size_t root, size_t length)
{
    for (;;) {
        size_t child = root * 2 + 1;
        if (child >= length) {
            return;
        }
        if (child + 1 < length && array[child] < array[child + 1]) {
            ++child;
        }
        if (array[root] >= array[child]) {
            return;
        }
        sort_swap_int(&array[root], &array[child]);
        root = child;
    }
}

void sort_heap_range(int *array, size_t low, size_t high)
{
    size_t length = high - low;

    /* Build a max-heap bottom up: O(n) work. */
    for (size_t start = length / 2; start > 0; --start) {
        sift_down(array + low, start - 1, length);
    }

    /* Repeatedly move the maximum to the end and shrink the heap. */
    for (size_t end = length; end > 1; --end) {
        sort_swap_int(&array[low], &array[low + end - 1]);
        sift_down(array + low, 0, end - 1);
    }
}

SortStatus sort_heap(int *array, size_t length)
{
    SortStatus status = sort_check_arguments(array, length);
    if (status != SORT_OK) {
        return status;
    }

    sort_heap_range(array, 0, length);
    return SORT_OK;
}
