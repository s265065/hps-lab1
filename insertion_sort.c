#include <stdint-gcc.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "basic.h"

void insertionSort(uint32_t *arr, int64_t size) {
    int i, key, j;
    for (i = 1; i < size; i++) {
        key = arr[i];
        j = i - 1;

        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}


int32_t main(int32_t argc, char **argv) {
    int64_t elements = ELEM_NUM;
    int64_t cycles = CYCLE_NUM;
    if (argc > 1) {
        elements = strtol(argv[1], NULL, 10);
    }
    if (argc > 2) {
        cycles = strtol(argv[2], NULL, 10);
    }
    test_sort(cycles, elements, insertionSort, "insertion_sort_split", true);
    test_sort(cycles, elements, insertionSort, "insertion_sort", false);
    return 0;
}