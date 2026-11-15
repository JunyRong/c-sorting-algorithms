/*
 * sort_registry.c - documented properties of the available algorithms.
 */
#include <stddef.h>
#include <string.h>

#include "sort.h"

static const SortAlgorithmInfo kAlgorithms[] = {
    {
        "bubble",
        sort_bubble,
        "n",
        "n^2",
        "n^2",
        "1",
        1,
        1
    },
    {
        "cocktail",
        sort_cocktail,
        "n",
        "n^2",
        "n^2",
        "1",
        1,
        1
    },
    {
        "selection",
        sort_selection,
        "n^2",
        "n^2",
        "n^2",
        "1",
        0,
        1
    },
    {
        "insertion",
        sort_insertion,
        "n",
        "n^2",
        "n^2",
        "1",
        1,
        1
    },
    {
        "shell",
        sort_shell,
        "n log n",
        "n^1.25 (empirical)",
        "n^1.5",
        "1",
        0,
        1
    },
    {
        "merge",
        sort_merge,
        "n log n",
        "n log n",
        "n log n",
        "n",
        1,
        0
    },
    {
        "quick",
        sort_quick,
        "n log n",
        "n log n",
        "n log n",
        "log n",
        0,
        1
    },
    {
        "heap",
        sort_heap,
        "n log n",
        "n log n",
        "n log n",
        "1",
        0,
        1
    },
    {
        "counting",
        sort_counting,
        "n + k",
        "n + k",
        "n + k",
        "n + k",
        1,
        0
    },
    {
        "radix",
        sort_radix,
        "d(n + b)",
        "d(n + b)",
        "d(n + b)",
        "n + b",
        1,
        0
    },
    {
        "bucket",
        sort_bucket,
        "n + k",
        "n + k (uniform keys)",
        "n^2 (clustered keys)",
        "n + k",
        1,
        0
    }
};

size_t sort_algorithm_count(void)
{
    return sizeof kAlgorithms / sizeof kAlgorithms[0];
}

const SortAlgorithmInfo *sort_algorithm_at(size_t index)
{
    if (index >= sort_algorithm_count()) {
        return NULL;
    }
    return &kAlgorithms[index];
}

const SortAlgorithmInfo *sort_algorithm_find(const char *name)
{
    if (name == NULL) {
        return NULL;
    }
    for (size_t index = 0; index < sort_algorithm_count(); ++index) {
        if (strcmp(kAlgorithms[index].name, name) == 0) {
            return &kAlgorithms[index];
        }
    }
    return NULL;
}
