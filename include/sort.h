/*
 * sort.h - unified interface for the C sorting algorithm module.
 *
 * Shared conventions for every sort_* function:
 *   - length == 0 may be combined with array == NULL and returns SORT_OK.
 *   - length > 0 with array == NULL returns SORT_INVALID_ARGUMENT.
 *   - length < 2 returns SORT_OK and does not touch the array.
 *   - The element type is int; element counts use size_t.
 *   - On any result other than SORT_OK the array is left unmodified.
 */
#ifndef SORT_ALGORITHMS_H
#define SORT_ALGORITHMS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * sort_counting() allocates one counter per value in [min, max]. Spans larger
 * than this limit return SORT_RANGE_ERROR instead of attempting an unbounded
 * allocation. Override at build time with -DSORT_COUNTING_MAX_RANGE=<value>.
 */
#ifndef SORT_COUNTING_MAX_RANGE
#define SORT_COUNTING_MAX_RANGE 4194304u
#endif

/*
 * Upper bound on the number of buckets sort_bucket() creates (one bucket per
 * element, capped). Bucket sorting also needs O(length) scratch space.
 */
#ifndef SORT_BUCKET_MAX_COUNT
#define SORT_BUCKET_MAX_COUNT 65536u
#endif

typedef enum {
    SORT_OK = 0,
    SORT_INVALID_ARGUMENT,
    SORT_OUT_OF_MEMORY,
    SORT_RANGE_ERROR
} SortStatus;

/* Human readable name of a status; never returns NULL. */
const char *sort_status_string(SortStatus status);

SortStatus sort_bubble(int *array, size_t length);
SortStatus sort_cocktail(int *array, size_t length);
SortStatus sort_selection(int *array, size_t length);
SortStatus sort_insertion(int *array, size_t length);
SortStatus sort_shell(int *array, size_t length);
SortStatus sort_merge(int *array, size_t length);
SortStatus sort_quick(int *array, size_t length);
SortStatus sort_heap(int *array, size_t length);
SortStatus sort_counting(int *array, size_t length);
SortStatus sort_radix(int *array, size_t length);
SortStatus sort_bucket(int *array, size_t length);

typedef SortStatus (*SortFn)(int *array, size_t length);

/* Documented properties of one algorithm; stable/in_place use 1 for true. */
typedef struct {
    const char *name;
    SortFn function;
    const char *best_time;
    const char *average_time;
    const char *worst_time;
    const char *extra_space;
    int stable;
    int in_place;
} SortAlgorithmInfo;

/* Read-only registry of every algorithm in this module. */
size_t sort_algorithm_count(void);
const SortAlgorithmInfo *sort_algorithm_at(size_t index);
const SortAlgorithmInfo *sort_algorithm_find(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* SORT_ALGORITHMS_H */
