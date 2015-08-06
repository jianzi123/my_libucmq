#pragma once

// #include <stdio.h>
// #include <stdlib.h>
// #include <sys/types.h>
#include <limits.h>

#include "leveldb/c.h"

struct _leveldb_stuff {
    leveldb_t* db;
    leveldb_options_t *options;
    leveldb_readoptions_t *roptions;
    leveldb_writeoptions_t *woptions;
    leveldb_writebatch_t *wbatch;       // not safe.
    char dbname[PATH_MAX];
};

/*
 * Initial or create a level-db instance.
 */
struct _leveldb_stuff *ldb_initialize(const char *name, size_t block_size, size_t wb_size, size_t lru_size, short bloom_size);

/*
 * Close the level-db instance.
 */
void ldb_close(struct _leveldb_stuff *ldbs);

/*
 * Destroy the level-db.
 */
void ldb_destroy(struct _leveldb_stuff *ldbs);

/*
 * Get key's value.
 */
char *ldb_get(struct _leveldb_stuff *ldbs, const char *key, size_t klen, int *vlen);

/*
 * Set record (key value).
 */
int ldb_put(struct _leveldb_stuff *ldbs, const char *key, size_t klen, const char *value, size_t vlen);

/*
 * Push record (key value).
 */
#define ldb_push	ldb_put

/*
 * Pull record (key value).
 */
int ldb_pull(struct _leveldb_stuff *ldbs, const char *key, size_t klen, char *value, size_t vlen);

/*
 * Delete record.
 */
int ldb_delete(struct _leveldb_stuff *ldbs, const char *key, size_t klen);

/*
 * Batch set record.
 */
int ldb_batch_put(struct _leveldb_stuff *ldbs, const char *key, size_t klen, const char *value, size_t vlen);

/*
 * Batch del record.
 */
int ldb_batch_delete(struct _leveldb_stuff *ldbs, const char *key, size_t klen);

/*
 * Commit batch set.
 * With ldb_batch_put to use.
 */
int ldb_batch_commit(struct _leveldb_stuff *ldbs);

/*
 * Returns if key exists.
 */
int ldb_exists(struct _leveldb_stuff *ldbs, const char *key, size_t klen);

/*
 * compact.
 */
void ldb_compact(struct _leveldb_stuff *ldbs);
