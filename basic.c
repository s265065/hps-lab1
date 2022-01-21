#include <omp.h>
#include <malloc.h>
#include <string.h>
#include <bits/types/clock_t.h>
#include <bits/types/struct_timeval.h>
#include <stdlib.h>
#include <float.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include "basic.h"


int64_t **divide_by_parts(int64_t size, int32_t parts) {
    int64_t index = 0;
    int64_t **arr_data = calloc(parts, sizeof(void *));
    for (int i = 0; i < parts; ++i) {
        arr_data[i] = calloc(2, sizeof(int64_t));
    }
    int64_t part_size = size / parts;
    for (int i = 0; i < parts - 1; ++i) {
        arr_data[i][0] = part_size;
        arr_data[i][1] = index;
        index += part_size;
        size -= part_size;
    }
    arr_data[parts - 1][0] = size;
    arr_data[parts - 1][1] = index;
    return arr_data;
}

uint32_t min_of_arr(const uint32_t *arr, uint32_t size, const uint32_t *indexes, const uint32_t *sizes) {
    uint32_t min = INT32_MAX;
    uint32_t index = 0;
    for (int i = 0; i < size; ++i) {
        if (indexes[i] < sizes[i]) {
            if (arr[i] < min) {
                min = arr[i];
                index = i;
            }
        }
    }
    return index;
}

void merge_parts(uint32_t *dest_array, int32_t parts_count, uint32_t **arr_parts, int64_t **sizes_and_indexes,
                 int64_t total_size) {
    uint32_t next_nums[parts_count];
    uint32_t indexes[parts_count];
    uint32_t sizes[parts_count];
    memset(indexes, 0, parts_count * sizeof(uint32_t));
    for (int i = 0; i < parts_count; ++i) {
        next_nums[i] = arr_parts[i][0];
        sizes[i] = sizes_and_indexes[i][0];
    }
    for (int i = 0; i < total_size; ++i) {
        uint32_t min_num_index = min_of_arr(next_nums, parts_count, indexes, sizes);
        dest_array[i] = arr_parts[min_num_index][indexes[min_num_index]];
        indexes[min_num_index] += 1;

        next_nums[min_num_index] = arr_parts[min_num_index][indexes[min_num_index]];
    }
}

void parallel_sort_by_parts(uint32_t *arr, const int64_t size, void (*sort)(uint32_t *, int64_t)) {
    if (omp_get_max_threads() > 1) {
        uint32_t **arr_parts;
        int32_t parts_count = omp_get_max_threads();
        int64_t **sizes = divide_by_parts(size, parts_count);

        arr_parts = calloc(sizeof(void *), parts_count);
        for (int i = 0; i < parts_count; ++i) {
            arr_parts[i] = calloc(sizeof(uint32_t), sizes[i][0]);
            memcpy(arr_parts[i], arr + sizes[i][1], sizes[i][0] * sizeof(uint32_t));
        }
        for (int i = 0; i < parts_count; ++i) {
#pragma omp task default(none) shared(arr, sizes, i, arr_parts, sort)
            sort(arr_parts[i], sizes[i][0]);
        }
#pragma omp taskwait

        merge_parts(arr, parts_count, arr_parts, sizes, size);
        for (int i = 0; i < parts_count; ++i) {
            free(arr_parts[i]);
            free(sizes[i]);
        }
        free(arr_parts);
    } else {
        sort(arr, size);
    }
}

uint32_t *generate_rand_numbers(uint32_t count) {
    uint32_t *result = calloc(count, sizeof(uint32_t));
    for (int i = 0; i < count; ++i) {
        result[i] = rand();
    }
    return result;
}

int comparator(const void *val1, const void *val2) {
    return (*(int32_t *) val1) - (*(int32_t *) val2);
}

void test_sort(int64_t cycles, int64_t elements, void (*sort)(uint32_t *, int64_t), char *sort_name, bool arr_split) {
    printf("Preparing %ld elements for %s\n", elements, sort_name);
    uint32_t *src_arr = generate_rand_numbers(elements);
    uint32_t *arr_sorted = calloc(elements, sizeof(uint32_t));
    memcpy(arr_sorted, src_arr, sizeof(uint32_t) * elements);
    qsort(arr_sorted, elements, sizeof(uint32_t), comparator);
    uint32_t *arr_to_sort = calloc(elements, sizeof(uint32_t));

    int64_t err_count = 0;
    double time_sum = 0;
    long double real_time_sum = 0;
    printf("Starting %ld sorts of %ld elements. Sort type: %s\n", cycles, elements, sort_name);
#pragma omp parallel default(none) shared(cycles, arr_to_sort, src_arr, elements, arr_sorted, err_count, stderr, time_sum, real_time_sum, sort, arr_split) if(omp_get_max_threads() > 1)
    {
#pragma omp single
        {
            struct timeval tv1, tv2;
            clock_t start_t;
            clock_t end_t;
            double cpu_time;
            double real_time;
            for (int i = 0; i < cycles; ++i) {
                bool err = false;
                memcpy(arr_to_sort, src_arr, sizeof(uint32_t) * elements);
                start_t = clock();
                gettimeofday(&tv1, NULL);
                if (arr_split) {
                    parallel_sort_by_parts(arr_to_sort, elements, sort);
                } else {
                    sort(arr_to_sort, elements);
                }
                end_t = clock();
                gettimeofday(&tv2, NULL);
                if (memcmp(arr_to_sort, arr_sorted, sizeof(uint32_t) * elements) != 0) {
                    err = true;
                }
                cpu_time = (double) (end_t - start_t) / CLOCKS_PER_SEC;

                real_time = (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec);

                if (err) {
                    err_count++;
                    fprintf(stderr,
                            "%d sort is done with error for %.2fs and used %.2fs of cpu time\n",
                            i + 1, real_time, cpu_time);
                } else {
                    printf("%d sort is done for %.2fs and used %.2fs of cpu time\n",
                           i + 1, real_time, cpu_time);
                }
                time_sum += cpu_time;
                real_time_sum += real_time;
            }
        }
    }
    free(src_arr);
    free(arr_to_sort);
    free(arr_sorted);

    puts("___________________________");
    printf("Mean time is: %Lf seconds\n", real_time_sum / cycles);

    printf("%ld sorts of %ld elements took %Lfs of real time and %fs of cpu time\n", cycles, elements, real_time_sum,
           time_sum);
    printf("%ld (%Lf%%) sorts failed\n", err_count, (((long double) err_count) / cycles) * 100);
}
