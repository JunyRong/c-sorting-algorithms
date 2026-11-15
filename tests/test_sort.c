/*
 * test_sort.c - self-contained test suite for the sorting module.
 *
 * Every algorithm is checked against identical inputs:
 *   - empty input (both NULL/0 and a valid pointer with length 0)
 *   - NULL with a non-zero length -> SORT_INVALID_ARGUMENT
 *   - one and two elements, already sorted, reversed, all equal
 *   - duplicates, negatives, INT_MIN/INT_MAX
 *   - generated ascending, descending, paired, sawtooth, binary and random data
 *
 * Results are compared against qsort(), so the checks cover both "sorted" and
 * "is a permutation of the input". Guard values around the allocation detect
 * out-of-bounds writes.
 */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sort.h"

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof((array)[0]))
#define CANARY 0x5A5A5A5A
#define MAX_REPORTED_FAILURES 40ul

typedef struct {
    const char *name;
    SortFn function;
    size_t stress_length;
    int range_guard;
} TestTarget;

static unsigned long g_checks;
static unsigned long g_failures;
static unsigned long g_reported;
static unsigned long g_random_state = 20240517ul;

static const int kSingle[] = {42};
static const int kTwoUnsorted[] = {2, 1};
static const int kTwoSorted[] = {-3, 5};
static const int kAscending[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
static const int kDescending[] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
static const int kAllEqual[] = {7, 7, 7, 7, 7, 7, 7, 7, 7};
static const int kDuplicates[] = {3, 1, 2, 3, 1, 2, 2, 3, 1, 0, 0, 3, 2};
static const int kNegatives[] = {0, -1, -5, 3, -5, 2, -9, 8, -9, 4, -1};
static const int kExtremes[] = {
    INT_MAX, INT_MIN, 0, 1, -1, INT_MAX, INT_MIN, 0, INT_MIN + 1, INT_MAX - 1
};

static void record_failure(const char *algorithm, const char *case_name,
                           const char *detail)
{
    ++g_failures;
    if (g_reported < MAX_REPORTED_FAILURES) {
        ++g_reported;
        printf("  FAIL [%s] %s: %s\n", algorithm, case_name, detail);
    }
}

static void check(int condition, const char *algorithm, const char *case_name,
                  const char *detail)
{
    ++g_checks;
    if (!condition) {
        record_failure(algorithm, case_name, detail);
    }
}

static int compare_int(const void *lhs, const void *rhs)
{
    int left = *(const int *)lhs;
    int right = *(const int *)rhs;
    return (left > right) - (left < right);
}

static int random_between(int low, int high)
{
    unsigned long long span = (unsigned long long)((long long)high - (long long)low) + 1ull;
    g_random_state = g_random_state * 1103515245ul + 12345ul;
    unsigned long long offset = ((unsigned long long)(g_random_state >> 16) * 2654435761ull) % span;
    return (int)((long long)low + (long long)offset);
}

static SortStatus expected_status(const TestTarget *target, const int *input,
                                  size_t length)
{
    if (length > 0 && input == NULL) {
        return SORT_INVALID_ARGUMENT;
    }
    if (target->range_guard && length > 1) {
        long long lowest = input[0];
        long long highest = input[0];
        for (size_t index = 1; index < length; ++index) {
            if (input[index] < lowest) {
                lowest = input[index];
            }
            if (input[index] > highest) {
                highest = input[index];
            }
        }
        unsigned long long span = (unsigned long long)(highest - lowest) + 1ull;
        if (span > (unsigned long long)SORT_COUNTING_MAX_RANGE) {
            return SORT_RANGE_ERROR;
        }
    }
    return SORT_OK;
}

static void verify_result(const TestTarget *target, const char *case_name,
                          const int *input, const int *result, size_t length)
{
    char detail[256];
    size_t index = 1;

    while (index < length && result[index - 1] <= result[index]) {
        ++index;
    }
    if (index < length) {
        snprintf(detail, sizeof detail, "not sorted at index %lu: %d before %d",
                 (unsigned long)index, result[index - 1], result[index]);
        record_failure(target->name, case_name, detail);
        return;
    }

    if (length == 0) {
        return;
    }

    int *reference = (int *)malloc(length * sizeof *reference);
    if (reference == NULL) {
        record_failure(target->name, case_name, "allocation failed");
        return;
    }
    memcpy(reference, input, length * sizeof *reference);
    qsort(reference, length, sizeof *reference, compare_int);

    if (memcmp(reference, result, length * sizeof *result) != 0) {
        record_failure(target->name, case_name,
                       "result is not a permutation of the input");
    }
    free(reference);
}

static void run_case(const TestTarget *target, const char *case_name,
                     const int *input, size_t length)
{
    char detail[256];
    SortStatus expected = expected_status(target, input, length);

    if (length > 0 && input == NULL) {
        SortStatus actual = target->function(NULL, length);
        if (actual != expected) {
            snprintf(detail, sizeof detail, "expected %s but got %s",
                     sort_status_string(expected), sort_status_string(actual));
            check(0, target->name, case_name, detail);
        } else {
            check(1, target->name, case_name, "");
        }
        return;
    }

    int *raw = NULL;
    int *work = NULL;
    if (length > 0) {
        raw = (int *)malloc((length + 2) * sizeof *raw);
        if (raw == NULL) {
            record_failure(target->name, case_name, "allocation failed");
            return;
        }
        raw[0] = CANARY;
        raw[length + 1] = CANARY;
        work = raw + 1;
        memcpy(work, input, length * sizeof *work);
    }

    SortStatus actual = target->function(work, length);
    if (actual != expected) {
        snprintf(detail, sizeof detail, "expected %s but got %s",
                 sort_status_string(expected), sort_status_string(actual));
        check(0, target->name, case_name, detail);
    } else {
        check(1, target->name, case_name, "");
        if (expected == SORT_OK) {
            verify_result(target, case_name, input, work, length);
        } else if (length > 0 &&
                   memcmp(work, input, length * sizeof *work) != 0) {
            record_failure(target->name, case_name,
                           "array was modified even though the call failed");
        }
    }

    if (raw != NULL) {
        if (raw[0] != CANARY || raw[length + 1] != CANARY) {
            record_failure(target->name, case_name,
                           "wrote outside the array bounds");
        }
        free(raw);
    }
}

enum Pattern {
    PATTERN_ASCENDING,
    PATTERN_DESCENDING,
    PATTERN_PAIRS,
    PATTERN_SAWTOOTH,
    PATTERN_BINARY,
    PATTERN_ALL_EQUAL
};

static void run_pattern_case(const TestTarget *target, const char *case_name,
                             size_t length, enum Pattern pattern)
{
    int *values = (int *)malloc(length * sizeof *values);
    if (values == NULL) {
        record_failure(target->name, case_name, "allocation failed");
        return;
    }

    for (size_t index = 0; index < length; ++index) {
        switch (pattern) {
        case PATTERN_ASCENDING:
            values[index] = (int)index;
            break;
        case PATTERN_DESCENDING:
            values[index] = (int)(length - 1 - index);
            break;
        case PATTERN_PAIRS:
            values[index] = (int)(index / 2);
            break;
        case PATTERN_SAWTOOTH:
            values[index] = (int)(index % 10) - 5;
            break;
        case PATTERN_BINARY:
            values[index] = (int)(index % 2) * 3;
            break;
        case PATTERN_ALL_EQUAL:
            values[index] = 7;
            break;
        }
    }

    run_case(target, case_name, values, length);
    free(values);
}

static void run_random_case(const TestTarget *target, const char *case_name,
                            size_t length, int low, int high)
{
    int *values = (int *)malloc(length * sizeof *values);
    if (values == NULL) {
        record_failure(target->name, case_name, "allocation failed");
        return;
    }
    for (size_t index = 0; index < length; ++index) {
        values[index] = random_between(low, high);
    }
    run_case(target, case_name, values, length);
    free(values);
}

static void run_algorithm_tests(const TestTarget *target)
{
    char name[96];

    run_case(target, "null-empty", NULL, 0);
    run_case(target, "null-nonempty", NULL, 3);

    {
        int placeholder = 0;
        run_case(target, "empty-array", &placeholder, 0);
    }

    run_case(target, "single", kSingle, ARRAY_LENGTH(kSingle));
    run_case(target, "two-unsorted", kTwoUnsorted, ARRAY_LENGTH(kTwoUnsorted));
    run_case(target, "two-sorted", kTwoSorted, ARRAY_LENGTH(kTwoSorted));
    run_case(target, "ascending", kAscending, ARRAY_LENGTH(kAscending));
    run_case(target, "descending", kDescending, ARRAY_LENGTH(kDescending));
    run_case(target, "all-equal", kAllEqual, ARRAY_LENGTH(kAllEqual));
    run_case(target, "duplicates", kDuplicates, ARRAY_LENGTH(kDuplicates));
    run_case(target, "negatives", kNegatives, ARRAY_LENGTH(kNegatives));
    run_case(target, "integer-extremes", kExtremes, ARRAY_LENGTH(kExtremes));

    snprintf(name, sizeof name, "ascending-%lu",
             (unsigned long)target->stress_length);
    run_pattern_case(target, name, target->stress_length, PATTERN_ASCENDING);

    snprintf(name, sizeof name, "descending-%lu",
             (unsigned long)target->stress_length);
    run_pattern_case(target, name, target->stress_length, PATTERN_DESCENDING);

    snprintf(name, sizeof name, "pairs-%lu",
             (unsigned long)target->stress_length);
    run_pattern_case(target, name, target->stress_length, PATTERN_PAIRS);

    snprintf(name, sizeof name, "sawtooth-%lu",
             (unsigned long)target->stress_length);
    run_pattern_case(target, name, target->stress_length, PATTERN_SAWTOOTH);

    snprintf(name, sizeof name, "binary-%lu",
             (unsigned long)target->stress_length);
    run_pattern_case(target, name, target->stress_length, PATTERN_BINARY);

    snprintf(name, sizeof name, "all-equal-%lu",
             (unsigned long)target->stress_length);
    run_pattern_case(target, name, target->stress_length, PATTERN_ALL_EQUAL);

    snprintf(name, sizeof name, "random-wide-%lu",
             (unsigned long)target->stress_length);
    run_random_case(target, name, target->stress_length, -100000, 100000);

    snprintf(name, sizeof name, "random-narrow-%lu",
             (unsigned long)target->stress_length);
    run_random_case(target, name, target->stress_length, -3, 3);

    snprintf(name, sizeof name, "random-extremes-%lu",
             (unsigned long)target->stress_length);
    run_random_case(target, name, target->stress_length, INT_MIN, INT_MAX);
}

static void verify_registry(void)
{
    size_t count = sort_algorithm_count();
    char detail[256];

    check(count > 0, "registry", "count", "no algorithms registered");

    for (size_t index = 0; index < count; ++index) {
        const SortAlgorithmInfo *info = sort_algorithm_at(index);
        if (info == NULL) {
            record_failure("registry", "entry", "sort_algorithm_at returned NULL");
            continue;
        }
        check(info->name != NULL && info->name[0] != '\0', "registry", "name",
              "algorithm name is empty");
        check(info->function != NULL, "registry", "function",
              "algorithm function is NULL");
        check(info->best_time != NULL && info->average_time != NULL &&
                  info->worst_time != NULL && info->extra_space != NULL,
              "registry", "complexity", "complexity description is missing");
        check(sort_algorithm_find(info->name) == info, "registry", "lookup",
              "sort_algorithm_find did not return the matching entry");

        for (size_t other = index + 1; other < count; ++other) {
            const SortAlgorithmInfo *candidate = sort_algorithm_at(other);
            if (candidate != NULL && strcmp(info->name, candidate->name) == 0) {
                snprintf(detail, sizeof detail, "duplicate name \"%s\"", info->name);
                check(0, "registry", "unique", detail);
            }
        }
    }

    check(sort_algorithm_at(count) == NULL, "registry", "bounds",
          "sort_algorithm_at(count) should return NULL");
    check(sort_algorithm_find("no-such-algorithm") == NULL, "registry", "lookup",
          "unknown name should return NULL");
    check(sort_algorithm_find(NULL) == NULL, "registry", "lookup",
          "NULL name should return NULL");

    check(strcmp(sort_status_string(SORT_OK), "SORT_OK") == 0, "registry",
          "status", "SORT_OK name mismatch");
    check(strcmp(sort_status_string(SORT_INVALID_ARGUMENT),
                 "SORT_INVALID_ARGUMENT") == 0,
          "registry", "status", "SORT_INVALID_ARGUMENT name mismatch");
    check(strcmp(sort_status_string(SORT_OUT_OF_MEMORY),
                 "SORT_OUT_OF_MEMORY") == 0,
          "registry", "status", "SORT_OUT_OF_MEMORY name mismatch");
    check(strcmp(sort_status_string(SORT_RANGE_ERROR), "SORT_RANGE_ERROR") == 0,
          "registry", "status", "SORT_RANGE_ERROR name mismatch");
    check(sort_status_string((SortStatus)999) != NULL, "registry", "status",
          "unknown status should still return a message");
}

static size_t stress_length_for(const char *name)
{
    static const struct {
        const char *name;
        size_t length;
    } kStress[] = {
        {"bubble", 1000},   {"cocktail", 1000}, {"selection", 1000},
        {"insertion", 1000}, {"shell", 4000},   {"merge", 20000},
        {"quick", 20000},   {"heap", 20000},    {"counting", 20000},
        {"radix", 20000},   {"bucket", 20000}
    };

    for (size_t index = 0; index < ARRAY_LENGTH(kStress); ++index) {
        if (strcmp(kStress[index].name, name) == 0) {
            return kStress[index].length;
        }
    }
    return 1000;
}

static void print_complexity_table(void)
{
    printf("\nAlgorithm properties (as documented by the module)\n");
    printf("%-10s %-7s %-9s %-14s %-26s %-16s %s\n", "Name", "Stable",
           "In place", "Best", "Average", "Worst", "Extra space");

    for (size_t index = 0; index < sort_algorithm_count(); ++index) {
        const SortAlgorithmInfo *info = sort_algorithm_at(index);
        printf("%-10s %-7s %-9s %-14s %-26s %-16s %s\n", info->name,
               info->stable ? "yes" : "no", info->in_place ? "yes" : "no",
               info->best_time, info->average_time, info->worst_time,
               info->extra_space);
    }
}

int main(void)
{
    printf("C sorting algorithms - test suite\n");
    printf("Reference results are produced with qsort().\n\n");

    verify_registry();

    size_t count = sort_algorithm_count();
    for (size_t index = 0; index < count; ++index) {
        const SortAlgorithmInfo *info = sort_algorithm_at(index);
        if (info == NULL) {
            continue;
        }

        TestTarget target;
        target.name = info->name;
        target.function = info->function;
        target.stress_length = stress_length_for(info->name);
        target.range_guard = strcmp(info->name, "counting") == 0;

        unsigned long failures_before = g_failures;
        printf("Testing %-10s (stress length %lu)\n", info->name,
               (unsigned long)target.stress_length);
        run_algorithm_tests(&target);
        if (g_failures == failures_before) {
            printf("  ok\n");
        }
    }

    print_complexity_table();

    printf("\n%lu checks, %lu failure(s)\n", g_checks, g_failures);
    if (g_failures != 0) {
        printf("TEST RUN FAILED\n");
        return EXIT_FAILURE;
    }
    printf("ALL TESTS PASSED\n");
    return EXIT_SUCCESS;
}
