#ifndef SORT_H
#define SORT_H

#include <stddef.h>

void heap_sort(void *arr, size_t n, size_t elem_size,
               int (*compare)(const void *, const void *));

#endif
