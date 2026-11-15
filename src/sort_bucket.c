/*
 * sort_bucket.c - bucket (bin) sort for int values.
 *
 * Elements are distributed into value-range buckets, each bucket is finished
 * with insertion sort, and the buckets are concatenated. Uniformly distributed
 * input is roughly O(n + k); strongly clustered input degrades towards O(n^2).
 * Stable, not in place, O(n + k) extra space.
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "sort_internal.h"

static size_t bucket_index(int value, int minimum, int maximum, size_t bucket_count)
{
    if (value <= minimum) {
        return 0;
    }
    if (value >= maximum) {
        return bucket_count - 1;
    }

    long double offset = (long double)value - (long double)minimum;
    long double span = (long double)maximum - (long double)minimum;
    long double scaled = offset * (long double)bucket_count / span;

    size_t index = (size_t)scaled;
    if (index >= bucket_count) {
        index = bucket_count - 1;
    }
    return index;
}

SortStatus sort_bucket(int *array, size_t length)
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

    size_t bucket_count = length;
    if (bucket_count > (size_t)SORT_BUCKET_MAX_COUNT) {
        bucket_count = (size_t)SORT_BUCKET_MAX_COUNT;
    }
    if (bucket_count < 1) {
        bucket_count = 1;
    }

    int minimum = array[0];
    int maximum = array[0];
    for (size_t index = 1; index < length; ++index) {
        if (array[index] < minimum) {
            minimum = array[index];
        }
        if (array[index] > maximum) {
            maximum = array[index];
        }
    }
    if (minimum == maximum) {
        return SORT_OK;
    }

    size_t *bounds = (size_t *)malloc((bucket_count + 1) * sizeof *bounds);
    size_t *cursor = (size_t *)malloc(bucket_count * sizeof *cursor);
    int *scratch = (int *)malloc(length * sizeof *scratch);
    if (bounds == NULL || cursor == NULL || scratch == NULL) {
        free(bounds);
        free(cursor);
        free(scratch);
        return SORT_OUT_OF_MEMORY;
    }

    memset(bounds, 0, (bucket_count + 1) * sizeof *bounds);
    for (size_t index = 0; index < length; ++index) {
        size_t bucket = bucket_index(array[index], minimum, maximum, bucket_count);
        ++bounds[bucket + 1];
    }
    for (size_t bucket = 1; bucket <= bucket_count; ++bucket) {
        bounds[bucket] += bounds[bucket - 1];
    }
    memcpy(cursor, bounds, bucket_count * sizeof *cursor);

    /* Forward scatter keeps equal values in their original relative order. */
    for (size_t index = 0; index < length; ++index) {
        size_t bucket = bucket_index(array[index], minimum, maximum, bucket_count);
        scratch[cursor[bucket]++] = array[index];
    }

    for (size_t bucket = 0; bucket < bucket_count; ++bucket) {
        sort_insertion_range(scratch, bounds[bucket], bounds[bucket + 1]);
    }

    memcpy(array, scratch, length * sizeof *array);
    free(scratch);
    free(cursor);
    free(bounds);
    return SORT_OK;
}
