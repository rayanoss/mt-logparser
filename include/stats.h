#ifndef STATS_H
#define STATS_H

#include <stddef.h>
#include "parser.h"

#define MAX_STATUS 600

struct method_stats
{
    int counts[METHOD_COUNT];
    int total;
};
typedef struct method_stats method_stats_t;

struct status_stats
{
    int counts[MAX_STATUS];
    int total;
};
typedef struct status_stats status_stats_t;

typedef struct
{
    char *ip;
    int count;
} ip_data_t;

typedef struct
{
    ip_data_t *array;
    size_t index;
} collector_t;

void display_method_stats(method_stats_t *stats);
void display_status_stats(status_stats_t *stats);
void display_bandwidth_stats(int total_bandwidth, int total_requests);
void display_ip_stats(hash_t *ip_table, int total_requests);

void collect_entry(char *key, void *value, void *user_data);

int compare_ip_data(const void *a, const void *b);

void merge_ip_tables(hash_t *dest, hash_t *src);
void destroy_ip_table(hash_t *table);

#endif
