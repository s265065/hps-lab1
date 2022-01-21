#ifndef HPS1_BASIC_H
#define HPS1_BASIC_H
#define ELEM_NUM 50000000
#define CYCLE_NUM 10

#include <stdint-gcc.h>


void parallel_sort_by_parts(uint32_t *arr, int64_t size, void(*sort)(uint32_t *, int64_t));

void test_sort(int64_t cycles, int64_t elements, void (*sort)(uint32_t *, int64_t), char *sort_name, bool arr_split);

#endif //HPS1_BASIC_H
