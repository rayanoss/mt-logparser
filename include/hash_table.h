#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stddef.h>

typedef struct entry entry_t;
typedef struct hash_table hash_t;
typedef void (*iterate_callback)(char *key, void *value, void *user_data);

hash_t *hash_table_create(size_t capacity);
void insert(char *key, hash_t *hash_table, void *value);
entry_t *get(char *key, hash_t *hash_table);
void delete (char *key, hash_t *hash_table);
void destroy(hash_t *hash_table);
void iterate(hash_t *table, iterate_callback callback, void *user_data);
size_t get_count(hash_t *table);

#endif