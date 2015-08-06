/*
 * file: hashmap.c
 * date: 2014/12/13
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "hashmap.h"

#define DEFAULT_BUCKET_SIZE 16

// The FALLTHROUGH_INTENDED macro can be used to annotate implicit fall-through
// between switch labels. The real definition should be provided externally.
// This one is a fallback version for unsupported compilers.
#ifndef FALLTHROUGH_INTENDED
#define FALLTHROUGH_INTENDED do { } while (0)
#endif

inline static uint32_t DecodeFixed32(const char* ptr)
{
        uint32_t result;
        memcpy(&result, ptr, sizeof(result));  // gcc optimizes this to a plain load
        return result;
}

static uint32_t Hash(const char* data, size_t n, uint32_t seed) {
        // Similar to murmur hash
        const uint32_t m = 0xc6a4a793;
        const uint32_t r = 24;
        const char* limit = data + n;
        uint32_t h = seed ^ (n * m);

        // Pick up four bytes at a time
        while (data + 4 <= limit) {
                uint32_t w = DecodeFixed32(data);
                data += 4;
                h += w;
                h *= m;
                h ^= (h >> 16);
        }

        // Pick up remaining bytes
        switch (limit - data) {
                case 3:
                        h += data[2] << 16;
                        FALLTHROUGH_INTENDED;
                case 2:
                        h += data[1] << 8;
                        FALLTHROUGH_INTENDED;
                case 1:
                        h += data[0];
                        h *= m;
                        h ^= (h >> r);
                        break;
        }
        return h;
}

hashmap_t *hashmap_open(void)
{
        hashmap_t *hashmap = (hashmap_t *)malloc(sizeof(hashmap_t));
        hashmap->bucket_count = DEFAULT_BUCKET_SIZE;
        hashmap->node_count = 0;
        hashmap->bucket = (list_t *)calloc(hashmap->bucket_count, sizeof(list_t));

        return hashmap;
}

void hashmap_close(hashmap_t *hashmap)
{
        /* free nodes memory. */
        size_t i;
        node_t *cur, *next;
        for (i = 0; i < hashmap->bucket_count; ++i) {
                cur = hashmap->bucket[i].next;
                while (cur != NULL) {
                        next = cur->next;
                        free(cur);
                        cur = next;
                }
        }

        /* free buckets memory. */
        free(hashmap->bucket);

        /* free hash hashmap. */
        free(hashmap);
}

static node_t *find_prev_pointer(list_t *list, void *key, size_t klen, uint32_t hash)
{
        node_t *cur = list->next;
        node_t *prev = list;
        while (cur != NULL) {
                if (cur->hash == hash && cur->klen == klen && memcmp(cur->key, key, klen) == 0) {
                        return prev;
                }
                cur = cur->next;
                prev = prev->next;
        }
        return NULL;
}

void hashmap_set(hashmap_t *hashmap, void *key, size_t klen, void *val, size_t vlen)
{
        assert(klen <= HASH_MAP_MAX_KEY);

        uint32_t hc = Hash(key, klen, 0);
        list_t *cur_list = hashmap->bucket + (hc % hashmap->bucket_count);
        node_t *prev = find_prev_pointer(cur_list, key, klen, hc);
        if (prev != NULL) {
                node_t *node = prev->next;
                if (node->total >= vlen) {
                        memcpy(node->val, val, vlen);
                        node->vlen = vlen;
                        return;
                }
                prev->next = node->next;
                free(node);
        }

        prev = (node_t *)malloc(sizeof(node_t) + vlen);
        memcpy(prev->key, key, klen);
        memcpy(prev->val, val, vlen);
        prev->klen = klen;
        prev->vlen = vlen;
        prev->hash = hc;
        prev->total = vlen;
        prev->next = cur_list->next;
        cur_list->next = prev;

        ++ hashmap->node_count;
        // FIXME: rehash...
}

void *hashmap_get(hashmap_t *hashmap, void *key, size_t klen, void *val, size_t *vlen)
{
        uint32_t hc = Hash(key, klen, 0);
        list_t *cur_list = hashmap->bucket + (hc % hashmap->bucket_count);
        node_t *prev = find_prev_pointer(cur_list, key, klen, hc);
        if (prev != NULL) {
                *vlen = prev->next->vlen;
                if (val != NULL) {
                        memcpy(val, prev->next->val, prev->next->vlen);
                }
                return prev->next->val;
        }

        *vlen = 0;
        return NULL;
}

