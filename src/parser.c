#include "parser.h"
#include "stats.h"
#include <string.h>
#include <stdlib.h>

static enum http_method extract_method(char *ptr, char *nl)
{
    char *quote = memchr(ptr, '"', nl - ptr + 1);
    if (!quote)
        return METHOD_DELETE;

    char *method_start = quote + 1;
    char *method_end = memchr(method_start, ' ', nl - method_start + 1);
    if (!method_end)
        return METHOD_DELETE;

    size_t method_len = method_end - method_start;

    switch (method_len)
    {
    case 3:
        if (method_start[0] == 'G' &&
            method_start[1] == 'E' &&
            method_start[2] == 'T')
            return METHOD_GET;

        if (method_start[0] == 'P' &&
            method_start[1] == 'U' &&
            method_start[2] == 'T')
            return METHOD_PUT;
        break;
    case 4:
        if (method_start[0] == 'P' &&
            method_start[1] == 'O' &&
            method_start[2] == 'S' &&
            method_start[3] == 'T')
            return METHOD_POST;
        break;
    case 6:
        if (method_start[0] == 'D' &&
            method_start[1] == 'E' &&
            method_start[2] == 'L' &&
            method_start[3] == 'E' &&
            method_start[4] == 'T' &&
            method_start[5] == 'E')
            return METHOD_DELETE;
        break;

    default:
        break;
    }
    return METHOD_DELETE;
}

void parse_method(char *ptr, char *nl, method_stats_t *stats)
{
    enum http_method method = extract_method(ptr, nl);
    stats->counts[method]++;
    stats->total++;
}

static int extract_status_code(char *ptr, char *nl)
{
    char *quote1 = memchr(ptr, '"', nl - ptr);
    if (!quote1)
        return -1;

    char *quote2 = memchr(quote1 + 1, '"', nl - (quote1 + 1));
    if (!quote2)
        return -1;

    char *status_code_start = quote2 + 2;
    if (status_code_start + 2 >= nl)
        return -1;

    int status =
        (status_code_start[0] - '0') * 100 +
        (status_code_start[1] - '0') * 10 +
        (status_code_start[2] - '0');

    return status;
}

void parse_status_code(char *ptr, char *nl, status_stats_t *stats)
{
    int status = extract_status_code(ptr, nl);

    if (status >= 0 && status < 600)
    {
        stats->counts[status]++;
        stats->total++;
    }
}

void parse_ip(char *ptr, char *nl, hash_t *ip_table)
{
    char ip_buffer[16];
    char *address_end = memchr(ptr, ' ', nl - ptr + 1);
    if (!address_end)
        return;

    size_t len = address_end - ptr;
    if (len >= sizeof(ip_buffer))
        return;

    memcpy(ip_buffer, ptr, len);
    ip_buffer[len] = '\0';

    int *count = (int *)get(ip_buffer, ip_table);
    if (count == NULL)
    {
        count = malloc(sizeof(int));
        if (!count)
            return;
        *count = 1;
        insert(ip_buffer, ip_table, count);
    }
    else
    {
        (*count)++;
    }
}

void parse_bandwidth(char *ptr, char *nl, int *total_bandwidth)
{
    char *quote1 = memchr(ptr, '"', nl - ptr);
    if (!quote1)
        return;

    char *quote2 = memchr(quote1 + 1, '"', nl - (quote1 + 1));
    if (!quote2)
        return;

    char *status_code_start = quote2 + 2;
    char *space = memchr(status_code_start, ' ', nl - status_code_start);
    if (!space)
        return;

    char *bandwidth_start = space + 1;
    size_t len = nl - bandwidth_start;

    int bandwidth_value = 0;

    for (size_t i = 0; i < len; i++)
    {
        if (bandwidth_start[i] < '0' || bandwidth_start[i] > '9')
            break;
        bandwidth_value = bandwidth_value * 10 + (bandwidth_start[i] - '0');
    }
    *total_bandwidth += bandwidth_value;
}

const char *get_status_description(int code)
{
    switch (code)
    {
    case 200:
        return "OK";
    case 201:
        return "Created";
    case 204:
        return "No Content";
    case 301:
        return "Moved Permanently";
    case 302:
        return "Found";
    case 304:
        return "Not Modified";
    case 400:
        return "Bad Request";
    case 401:
        return "Unauthorized";
    case 403:
        return "Forbidden";
    case 404:
        return "Not Found";
    case 500:
        return "Internal Server Error";
    case 502:
        return "Bad Gateway";
    case 503:
        return "Service Unavailable";
    default:
        return "Unknown";
    }
}
