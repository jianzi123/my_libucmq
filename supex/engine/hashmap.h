/*
 * file: hashmap.h
 * date: 2014/12/12
 */

#include <stddef.h>
#include <stdint.h>

#define HASH_MAP_MAX_KEY 64

typedef struct _node
{
        struct _node *next;
        size_t klen;
        size_t vlen;
        uint32_t hash;
        uint32_t total; 
        char key[HASH_MAP_MAX_KEY];
        char val[0];
} node_t, list_t;

typedef struct _hashmap
{
        list_t *bucket;
        size_t bucket_count;
        size_t node_count;
} hashmap_t;
        
hashmap_t *hashmap_open(void);
void hashmap_close(hashmap_t *hashmap);
void hashmap_set(hashmap_t *hashmap, void *key, size_t klen, void *val, size_t vlen);
void *hashmap_get(hashmap_t *hashmap, void *key, size_t klen, void *val, size_t *vlen);
