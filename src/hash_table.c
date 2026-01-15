#define _POSIX_C_SOURCE 200809L

#include "hash_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

void resize(hash_t *hash_table);

struct entry
{
    char *key;
    void *value;
    struct entry *next;
};

struct hash_table
{
    entry_t **buckets;
    size_t capacity;
    size_t count;
};

hash_t *hash_table_create(size_t capacity)
{
    entry_t **entry_array = malloc(capacity * sizeof(entry_t *));
    if (!entry_array)
        return NULL;

    for (size_t i = 0; i < capacity; i++)
    {
        entry_array[i] = NULL;
    }

    hash_t *hash_table = malloc(sizeof(hash_t));
    if (!hash_table)
    {
        free(entry_array);
        return NULL;
    }

    hash_table->buckets = entry_array;
    hash_table->count = 0;
    hash_table->capacity = capacity;

    return hash_table;
}

size_t hash(char *key)
{
    size_t hash = 5381;

    while (*key)
    {
        hash = hash * 33 + *key;
        key++;
    }

    return hash;
}

void insert(char *key, hash_t *hash_table, void *value)
{
    if ((double)hash_table->count / hash_table->capacity > 0.7)
    {
        resize(hash_table);
    }

    size_t index = hash(key) % hash_table->capacity;
    entry_t *current = hash_table->buckets[index];

    while (current != NULL)
    {
        if (strcmp(current->key, key) == 0)
        {
            current->value = value;
            return;
        }
        current = current->next;
    }

    entry_t *new_entry = malloc(sizeof(entry_t));
    if (!new_entry)
        return;

    new_entry->key = strdup(key);
    if (!new_entry->key)
    {
        free(new_entry);
        return;
    }

    new_entry->value = value;
    new_entry->next = hash_table->buckets[index];

    hash_table->buckets[index] = new_entry;
    hash_table->count++;
};

entry_t *get(char *key, hash_t *hash_table)
{
    size_t index = hash(key) % hash_table->capacity;
    entry_t *current = hash_table->buckets[index];

    while (current != NULL)
    {
        if (strcmp(current->key, key) == 0)
        {

            return current->value;
        }
        current = current->next;
    }

    return NULL;
}

void delete(char *key, hash_t *hash_table)
{
    size_t index = hash(key) % hash_table->capacity;
    entry_t *current = hash_table->buckets[index];
    entry_t *previous = NULL;

    while (current != NULL)
    {
        if (strcmp(current->key, key) == 0)
        {
            if (previous == NULL)
            {
                hash_table->buckets[index] = current->next;
            }
            else
            {
                previous->next = current->next;
            }

            free(current->key);
            free(current);
            hash_table->count--;
            return;
        }
        previous = current;
        current = current->next;
    }
}

void destroy(hash_t *hash_table)
{
    for (size_t i = 0; i < hash_table->capacity; i++)
    {
        entry_t *current = hash_table->buckets[i];
        while (current != NULL)
        {
            entry_t *next = current->next;
            free(current->key);
            free(current);
            current = next;
        }
    }

    free(hash_table->buckets);
    free(hash_table);
}

size_t get_count(hash_t *table)
{
    return table->count;
}

void resize(hash_t *hash_table)
{
    entry_t **old_buckets = hash_table->buckets;
    size_t old_capacity = hash_table->capacity;

    hash_table->capacity = hash_table->capacity * 2;
    hash_table->buckets = malloc(hash_table->capacity * sizeof(entry_t *));
    if (!hash_table->buckets)
    {
        hash_table->buckets = old_buckets;
        hash_table->capacity = old_capacity;
        return;
    }

    for (size_t i = 0; i < hash_table->capacity; i++)
    {
        hash_table->buckets[i] = NULL;
    }

    hash_table->count = 0;

    for (size_t i = 0; i < old_capacity; i++)
    {
        entry_t *current = old_buckets[i];
        while (current != NULL)
        {
            entry_t *next = current->next;
            size_t index = hash(current->key) % hash_table->capacity;
            current->next = hash_table->buckets[index];
            hash_table->buckets[index] = current;
            hash_table->count++;
            current = next;
        }
    }

    free(old_buckets);
}

void iterate(hash_t *table, iterate_callback callback, void *user_data)
{
    for (size_t i = 0; i < table->capacity; i++){
        entry_t *current = table->buckets[i];
        while (current != NULL){
            callback(current->key, current->value, user_data); 
            current = current->next;
        }
    }
}