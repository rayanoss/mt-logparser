#include "stats.h"
#include "parser.h"
#include "hash_table.h"
#include "sort.h"
#include <stdio.h>
#include <stdlib.h>

static const char *method_names[METHOD_COUNT] = {
    [METHOD_GET] = "GET",
    [METHOD_POST] = "POST",
    [METHOD_PUT] = "PUT",
    [METHOD_DELETE] = "DELETE"
};

void display_method_stats(method_stats_t *stats)
{
    printf("─────────────────────────────────────\nHTTP METHODS\n─────────────────────────────────────\n");
    for (int i = 0; i < METHOD_COUNT; i++)
    {
        double pct = (double)stats->counts[i] / stats->total * 100.0;
        printf("%-10s %7d (%.1f%%)\n",
               method_names[i],
               stats->counts[i],
               pct);
    }
}

void display_status_stats(status_stats_t *stats)
{
    printf("─────────────────────────────────────\nSTATUS CODES\n─────────────────────────────────────\n");
    for (int i = 0; i < MAX_STATUS; i++)
    {
        if (stats->counts[i] > 0)
        {
            double pct = (double)stats->counts[i] / stats->total * 100.0;
            printf("%3d %-25s %7d (%.1f%%)\n",
                   i,
                   get_status_description(i),
                   stats->counts[i],
                   pct);
        }
    }
}

void display_bandwidth_stats(int total_bandwidth, int total_requests)
{
    printf("─────────────────────────────────────\nBANDWIDTH\n─────────────────────────────────────\n");
    printf("%-25s %10.1f MB\n", "Total transferred:", total_bandwidth / 1000000.0);
    printf("%-25s %10.1f KB\n", "Average per request:", (total_bandwidth / 1000.0) / total_requests);
}

void display_ip_stats(hash_t *ip_table, int total_requests)
{
    if (ip_table == NULL)
        return;

    size_t count = get_count(ip_table);
    if (count == 0)
        return;

    ip_data_t *data = malloc(count * sizeof(ip_data_t));
    if (!data)
        return;

    collector_t collector = {data, 0};

    iterate(ip_table, collect_entry, &collector);
    heap_sort(collector.array, collector.index, sizeof(ip_data_t), compare_ip_data);

    printf("─────────────────────────────────────\nTOP 10 IP ADDRESSES\n─────────────────────────────────────\n");
    for (size_t i = 0; i < 10 && i < count; i++)
    {
        double pct = (double)data[i].count / total_requests * 100.0;
        printf("%2zu. %-15s %7d requests (%.1f%%)\n",
               i + 1,
               data[i].ip,
               data[i].count,
               pct);
    }

    free(data);
}

void collect_entry(char *key, void *value, void *user_data)
{
    collector_t *collector = (collector_t *)user_data;
    collector->array[collector->index].ip = key;
    collector->array[collector->index].count = *(int *)value;
    collector->index++;
}

int compare_ip_data(const void *a, const void *b)
{
    ip_data_t *ip_a = (ip_data_t *)a;
    ip_data_t *ip_b = (ip_data_t *)b;
    return ip_b->count - ip_a->count;
}

static void merge_ip_callback(char *key, void *value, void *user_data)
{
    hash_t *dest = (hash_t *)user_data;
    int count_src = *(int *)value;

    int *existing = (int *)get(key, dest);

    if (existing != NULL)
    {
        *existing += count_src;
    }
    else
    {
        int *new_count = malloc(sizeof(int));
        if (!new_count)
            return;
        *new_count = count_src;
        insert(key, dest, new_count);
    }
}

void merge_ip_tables(hash_t *dest, hash_t *src)
{
    iterate(src, merge_ip_callback, dest);
}

static void free_ip_value(char *key, void *value, void *user_data)
{
    (void)key;
    (void)user_data;
    free(value);
}

void destroy_ip_table(hash_t *table)
{
    if (table == NULL)
        return;
    iterate(table, free_ip_value, NULL);
    destroy(table);
}
