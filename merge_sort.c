#include <omp.h>
#include <stdint-gcc.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "basic.h"


void merge(uint32_t *arr, int first, int last) {

    int i, j, k;
    int middle =  (first + last) / 2;
    int n1 = middle - first + 1;
    int n2 = last - middle;

    int L[n1], R[n2];

    for (i = 0; i < n1; i++)
        L[i] = arr[first + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[middle + 1 + j];

    i = 0;
    j = 0;
    k = first;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        }
        else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }

}

void mergeSort(uint32_t *arr, int first, int last) {
    if (first < last) {
        mergeSort(arr, first, (first + last) / 2);
        mergeSort(arr, (first + last) / 2 + 1, last);
        merge(arr, first, last);
    }
}

void mergeSortCrutch(uint32_t *arr, int64_t size){
    int first = 0;
    int last = size - 1;
    mergeSort(arr, first, last);
}

void parallelMergeSort(uint32_t *arr, int first, int last) {
    int middle = (first + last) / 2;
#pragma omp parallel sections
    {
#pragma omp section
        {
            mergeSort(arr, first, middle);
        }
#pragma omp section
        {
            mergeSort(arr, middle+1, last);
        }
    }
    merge(arr, first, last);
}

void parallelMergeSortCrutch(uint32_t *arr, int64_t size){
    int first = 0;
    int last = size - 1;
    parallelMergeSort(arr, first, last);
}


int main(int argc, char **argv) {
    int64_t elements = ELEM_NUM;
    int64_t cycles = CYCLE_NUM;
    if (argc > 1) {
        elements = strtol(argv[1], NULL, 10);
    }
    if (argc > 2) {
        cycles = strtol(argv[2], NULL, 10);
    }
    test_sort(cycles, elements, mergeSortCrutch, "merge_sort", false);
    test_sort(cycles, elements, mergeSortCrutch, "merge_sort_split", true);
    test_sort(cycles, elements, parallelMergeSortCrutch, "parallel_merge_sort", false);
    test_sort(cycles, elements, parallelMergeSortCrutch, "parallel_merge_sort_split", true);
    return 0;
}
