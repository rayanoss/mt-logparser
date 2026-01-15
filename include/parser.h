#ifndef PARSER_H
#define PARSER_H

#include "hash_table.h"

typedef struct method_stats method_stats_t;
typedef struct status_stats status_stats_t;

enum http_method
{
    METHOD_GET,
    METHOD_POST,
    METHOD_PUT,
    METHOD_DELETE,
    METHOD_COUNT
};

void parse_method(char *ptr, char *nl, method_stats_t *stats);
void parse_status_code(char *ptr, char *nl, status_stats_t *stats);
void parse_ip(char *ptr, char *nl, hash_t *ip_table);
void parse_bandwidth(char *ptr, char *nl, int *total_bandwidth);

const char *get_status_description(int code);

#endif
