#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>

#include "parser.h"
#include "stats.h"
#include "sort.h"
#include "hash_table.h"

#define THREAD_POOL_SIZE 4

pthread_t thread_pool[THREAD_POOL_SIZE];

typedef struct
{
    char *data;
    char *start;
    size_t size;
} mapped_file_t;

typedef struct
{
    method_stats_t method_stats;
    status_stats_t status_stats;
    hash_t *ip_table;
    int total_bandwidth;
    int total_requests;
} log_stats_t;

typedef struct
{
    mapped_file_t *file;
    char *start;
    char *end;
} t_parse_arg;

static mapped_file_t *open_and_map_file(const char *filename)
{
    int fd = open(filename, O_RDONLY);
    if (fd == -1)
        return NULL;

    struct stat sb;
    if (fstat(fd, &sb) != 0)
    {
        close(fd);
        return NULL;
    }

    if (sb.st_size == 0)
    {
        close(fd);
        return NULL;
    }

    size_t fsize = (size_t)sb.st_size;
    char *data = mmap(NULL, fsize, PROT_READ, MAP_SHARED, fd, 0);

    close(fd);

    if (data == MAP_FAILED)
    {
        return NULL;
    }

    mapped_file_t *file = malloc(sizeof(mapped_file_t));
    if (!file)
    {
        munmap(data, fsize);
        return NULL;
    }

    file->data = data;
    file->start = data;
    file->size = fsize;

    return file;
}

static void close_and_unmap_file(mapped_file_t *file)
{
    if (file)
    {
        if (file->start)
        {
            munmap(file->start, file->size);
        }
        free(file);
    }
}

static void parse_all_logs(mapped_file_t *file, log_stats_t *stats)
{
    char *ptr = file->data;
    char *end_file = file->data + file->size;

    while (ptr < end_file)
    {
        char *nl = memchr(ptr, '\n', end_file - ptr);
        if (!nl)
        {
            break;
        }
        stats->total_requests++;

        parse_method(ptr, nl, &stats->method_stats);
        parse_status_code(ptr, nl, &stats->status_stats);
        parse_ip(ptr, nl, stats->ip_table);
        parse_bandwidth(ptr, nl, &stats->total_bandwidth);

        ptr = nl + 1;
    }
}

static void *t_parse_log(void *arg)
{
    t_parse_arg *args = (t_parse_arg *)arg;

    log_stats_t *stats = malloc(sizeof(log_stats_t));
    if (!stats)
    {
        free(args);
        return NULL;
    }

    *stats = (log_stats_t){
        .method_stats = {{0}},
        .status_stats = {{0}},
        .ip_table = hash_table_create(1000),
        .total_bandwidth = 0,
        .total_requests = 0};

    if (!stats->ip_table)
    {
        free(stats);
        free(args);
        return NULL;
    }

    char *ptr = args->start;
    char *end_file = args->end;

    while (ptr < end_file)
    {
        char *nl = memchr(ptr, '\n', end_file - ptr);
        if (!nl)
        {
            break;
        }
        stats->total_requests++;

        parse_method(ptr, nl, &stats->method_stats);
        parse_status_code(ptr, nl, &stats->status_stats);
        parse_ip(ptr, nl, stats->ip_table);
        parse_bandwidth(ptr, nl, &stats->total_bandwidth);

        ptr = nl + 1;
    }

    free(args);
    return (void *)stats;
}

static void merge_stats(log_stats_t *dest, log_stats_t *src)
{
    for (int i = 0; i < METHOD_COUNT; i++)
    {
        dest->method_stats.counts[i] += src->method_stats.counts[i];
    }
    dest->method_stats.total += src->method_stats.total;

    for (int i = 0; i < MAX_STATUS; i++)
    {
        dest->status_stats.counts[i] += src->status_stats.counts[i];
    }
    dest->status_stats.total += src->status_stats.total;

    if (src->ip_table != NULL)
    {
        if (dest->ip_table == NULL)
        {
            dest->ip_table = hash_table_create(1000);
            if (!dest->ip_table)
                return;
        }
        merge_ip_tables(dest->ip_table, src->ip_table);
    }

    dest->total_bandwidth += src->total_bandwidth;
    dest->total_requests += src->total_requests;
}

int main(void)
{
    struct timeval t1, t2;
    double time_no_threads, time_with_threads;

    mapped_file_t *file = open_and_map_file("./access.log");
    if (!file)
    {
        return 1;
    }

    gettimeofday(&t1, NULL);

    log_stats_t stats_no_threads = {
        .method_stats = {{0}},
        .status_stats = {{0}},
        .ip_table = hash_table_create(1000),
        .total_bandwidth = 0,
        .total_requests = 0};

    if (!stats_no_threads.ip_table)
    {
        close_and_unmap_file(file);
        fprintf(stderr, "Error: failed to create IP hash table\n");
        return 1;
    }

    parse_all_logs(file, &stats_no_threads);

    gettimeofday(&t2, NULL);
    time_no_threads = (t2.tv_sec - t1.tv_sec) * 1000.0 + (t2.tv_usec - t1.tv_usec) / 1000.0;

    gettimeofday(&t1, NULL);

    size_t chunk_size = file->size / THREAD_POOL_SIZE;
    for (int i = 0; i < THREAD_POOL_SIZE; i++)
    {
        char *start = file->data + (i * chunk_size);
        char *end;

        if (i == THREAD_POOL_SIZE - 1)
        {
            end = file->data + file->size;
        }
        else
        {
            end = file->data + ((i + 1) * chunk_size);
            while (end < file->data + file->size && *end != '\n')
            {
                end++;
            }
            if (end < file->data + file->size)
            {
                end++;
            }
        }

        if (i != 0)
        {
            while (start < file->data + file->size && *start != '\n')
            {
                start++;
            }
            start++;
        }
        t_parse_arg *arg = malloc(sizeof(t_parse_arg));
        if (!arg)
        {
            fprintf(stderr, "Error: failed to allocate thread argument\n");
            continue;
        }
        arg->file = file;
        arg->start = start;
        arg->end = end;
        pthread_create(&thread_pool[i], NULL, t_parse_log, arg);
    }

    log_stats_t *thread_stats[THREAD_POOL_SIZE];

    for (int i = 0; i < THREAD_POOL_SIZE; i++)
    {
        void *result;
        pthread_join(thread_pool[i], &result);
        thread_stats[i] = (log_stats_t *)result;
    }

    log_stats_t global_stats = {
        .method_stats = {{0}},
        .status_stats = {{0}},
        .ip_table = NULL,
        .total_bandwidth = 0,
        .total_requests = 0};

    for (int i = 0; i < THREAD_POOL_SIZE; i++)
    {
        if (thread_stats[i] != NULL)
        {
            merge_stats(&global_stats, thread_stats[i]);
            destroy_ip_table(thread_stats[i]->ip_table);
            free(thread_stats[i]);
        }
    }

    gettimeofday(&t2, NULL);
    time_with_threads = (t2.tv_sec - t1.tv_sec) * 1000.0 + (t2.tv_usec - t1.tv_usec) / 1000.0;

    close_and_unmap_file(file);

    printf("Threaded Parsing Results\n");
    printf("========================\n");
    printf("Total requests: %d\n", global_stats.total_requests);
    printf("Total bandwidth: %d bytes\n\n", global_stats.total_bandwidth);

    display_ip_stats(global_stats.ip_table, global_stats.total_requests);
    display_status_stats(&global_stats.status_stats);
    display_method_stats(&global_stats.method_stats);
    display_bandwidth_stats(global_stats.total_bandwidth, global_stats.total_requests);

    printf("\nPerformance Statistics\n");
    printf("======================\n");
    printf("Without threads: %.3f ms\n", time_no_threads);
    printf("With threads (%d): %.3f ms\n", THREAD_POOL_SIZE, time_with_threads);
    printf("Speedup: %.2fx\n", time_no_threads / time_with_threads);

    destroy_ip_table(stats_no_threads.ip_table);
    destroy_ip_table(global_stats.ip_table);

    return 0;
}
