/*
 * sort_quick.c - iterative three-way quicksort with introsort safeguards.
 *
 * - Ninther (median of three medians) pivot selection for larger ranges,
 *   median-of-three for small ones.
 * - Three-way partitioning keeps all-equal and duplicate-heavy inputs fast.
 * - Ranges smaller than QUICK_SMALL are finished with insertion sort.
 * - The loop always continues with the smaller side and pushes the larger one,
 *   so the explicit stack stays O(log n) and the call stack cannot degenerate.
 * - Each range carries a depth budget of 2 * log2(n); when it is exhausted the
 *   range is finished with heapsort. That bounds the worst case at O(n log n)
 *   even for pivot patterns that defeat the sampling.
 * Not stable, in place, O(log n) extra space.
 */
#include "sort_internal.h"

#define QUICK_SMALL 16
#define QUICK_NINTHER_THRESHOLD 128
#define QUICK_STACK_CAPACITY 128

typedef struct {
    size_t low;
    size_t high;
    unsigned int budget;
} SortRange;

static int median_of_three(int first, int second, int third)
{
    if (first < second) {
        if (second < third) {
            return second;
        }
        return first < third ? third : first;
    }
    if (first < third) {
        return first;
    }
    return second < third ? third : second;
}

static int choose_pivot(const int *array, size_t low, size_t high)
{
    size_t length = high - low;

    if (length >= QUICK_NINTHER_THRESHOLD) {
        size_t step = length / 8;
        int first = median_of_three(array[low], array[low + step],
                                    array[low + 2 * step]);
        int middle = median_of_three(array[low + 3 * step],
                                     array[low + 4 * step],
                                     array[low + 5 * step]);
        int last = median_of_three(array[low + 6 * step],
                                   array[low + 7 * step], array[high - 1]);
        return median_of_three(first, middle, last);
    }

    return median_of_three(array[low], array[low + length / 2], array[high - 1]);
}

/* 2 * floor(log2(length)) + 1: enough partitioning steps for a balanced tree. */
static unsigned int depth_budget(size_t length)
{
    unsigned int levels = 0;
    while (length > 1) {
        length >>= 1;
        ++levels;
    }
    return levels * 2 + 1;
}

static void push_range(int *array, SortRange *stack, size_t *depth, size_t low,
                       size_t high, unsigned int budget)
{
    if (*depth < QUICK_STACK_CAPACITY) {
        stack[*depth].low = low;
        stack[*depth].high = high;
        stack[*depth].budget = budget;
        ++(*depth);
        return;
    }

    /* Defensive fallback: still produces a correct result if the bound is hit. */
    sort_heap_range(array, low, high);
}

SortStatus sort_quick(int *array, size_t length)
{
    SortStatus status = sort_check_arguments(array, length);
    if (status != SORT_OK) {
        return status;
    }
    if (length < 2) {
        return SORT_OK;
    }

    SortRange stack[QUICK_STACK_CAPACITY];
    size_t depth = 0;
    push_range(array, stack, &depth, 0, length, depth_budget(length));

    while (depth > 0) {
        --depth;
        size_t low = stack[depth].low;
        size_t high = stack[depth].high;
        unsigned int budget = stack[depth].budget;

        for (;;) {
            if (high - low <= QUICK_SMALL) {
                sort_insertion_range(array, low, high);
                break;
            }
            if (budget == 0) {
                sort_heap_range(array, low, high);
                break;
            }
            --budget;

            int pivot = choose_pivot(array, low, high);

            /* [low, less)  < pivot
             * [less, index) == pivot
             * [greater, high) > pivot
             */
            size_t less = low;
            size_t index = low;
            size_t greater = high;

            while (index < greater) {
                if (array[index] < pivot) {
                    sort_swap_int(&array[less], &array[index]);
                    ++less;
                    ++index;
                } else if (array[index] > pivot) {
                    --greater;
                    sort_swap_int(&array[index], &array[greater]);
                } else {
                    ++index;
                }
            }

            size_t left_size = less - low;
            size_t right_size = high - greater;

            if (left_size < right_size) {
                if (right_size > 1) {
                    push_range(array, stack, &depth, greater, high, budget);
                }
                high = less;
            } else {
                if (left_size > 1) {
                    push_range(array, stack, &depth, low, less, budget);
                }
                low = greater;
            }

            if (high - low < 2) {
                break;
            }
        }
    }

    return SORT_OK;
}
