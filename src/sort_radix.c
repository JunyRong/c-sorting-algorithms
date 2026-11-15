/*
 * sort_radix.c - LSD radix sort over the raw two's complement bit pattern.
 *
 * The sign bit is flipped before reading digits, which maps signed int order
 * onto unsigned order. That makes INT_MIN, INT_MAX and all negative values work
 * without signed arithmetic or overflow. Four 8-bit passes for 32-bit int,
 * each stable. Stable, not in place, O(n + 256) extra space.
 */
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "sort_internal.h"

#define RADIX_BITS 8u
#define RADIX_BUCKETS (1u << RADIX_BITS)

static unsigned int radix_key(int value)
{
    unsigned int bits = (unsigned int)sizeof(int) * CHAR_BIT;
    return ((unsigned int)value) ^ (1u << (bits - 1u));
}

SortStatus sort_radix(int *array, size_t length)
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

    int *buffer = (int *)malloc(length * sizeof *buffer);
    if (buffer == NULL) {
        return SORT_OUT_OF_MEMORY;
    }

    unsigned int total_bits = (unsigned int)sizeof(int) * CHAR_BIT;
    size_t counts[RADIX_BUCKETS];
    int *source = array;
    int *target = buffer;

    for (unsigned int shift = 0; shift < total_bits; shift += RADIX_BITS) {
        unsigned int remaining = total_bits - shift;
        unsigned int pass_bits = remaining < RADIX_BITS ? remaining : RADIX_BITS;
        unsigned int digit_mask = (1u << pass_bits) - 1u;

        memset(counts, 0, sizeof counts);
        for (size_t index = 0; index < length; ++index) {
            ++counts[(radix_key(source[index]) >> shift) & digit_mask];
        }

        size_t offset = 0;
        for (unsigned int digit = 0; digit < RADIX_BUCKETS; ++digit) {
            size_t count = counts[digit];
            counts[digit] = offset;
            offset += count;
        }

        for (size_t index = 0; index < length; ++index) {
            unsigned int digit = (radix_key(source[index]) >> shift) & digit_mask;
            target[counts[digit]++] = source[index];
        }

        int *swap = source;
        source = target;
        target = swap;
    }

    if (source != array) {
        memcpy(array, source, length * sizeof *array);
    }

    free(buffer);
    return SORT_OK;
}
