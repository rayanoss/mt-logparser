#include "sort.h"
#include <string.h>

static void heapify(void *arr, size_t n, size_t elem_size, size_t i,
                    int (*compare)(const void *, const void *))
{
    size_t largest = i;
    size_t left = 2 * i + 1;
    size_t right = 2 * i + 2;

    void *elem_largest = (char *)arr + largest * elem_size;
    void *elem_left = (char *)arr + left * elem_size;
    void *elem_right = (char *)arr + right * elem_size;

    if (left < n && compare(elem_left, elem_largest) > 0)
    {
        largest = left;
        elem_largest = (char *)arr + largest * elem_size;
    }

    if (right < n && compare(elem_right, elem_largest) > 0)
    {
        largest = right;
        elem_largest = (char *)arr + largest * elem_size;
    }

    if (largest != i)
    {
        void *elem_i = (char *)arr + i * elem_size;
        char tmp[elem_size];
        memcpy(tmp, elem_i, elem_size);
        memcpy(elem_i, elem_largest, elem_size);
        memcpy(elem_largest, tmp, elem_size);
        heapify(arr, n, elem_size, largest, compare);
    }
}

void heap_sort(void *arr, size_t n, size_t elem_size,
               int (*compare)(const void *, const void *))
{
    for (int i = n / 2 - 1; i >= 0; i--)
    {
        heapify(arr, n, elem_size, i, compare);
    }

    for (size_t i = n - 1; i > 0; i--)
    {
        char tmp[elem_size];
        void *first_elem = arr;
        void *last_elem = (char *)arr + i * elem_size;
        memcpy(tmp, first_elem, elem_size);
        memcpy(first_elem, last_elem, elem_size);
        memcpy(last_elem, tmp, elem_size);
        heapify(arr, i, elem_size, 0, compare);
    }
}
