/*
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ldb.h"

struct _leveldb_stuff *g_supex_ldbs = NULL;

/*
 * Open the specified database, if no exists, then create it.
 * Args:
 *      1. name: db name
 *      2. block_size: block size
 *      3. wb_size: write buffer size
 *      4. lru_size: lru cache size
 *      5. bloom_size: bloom key size
 * Return:
 *      _leveldb_stuff: leveldb handler.
 */
struct _leveldb_stuff *ldb_initialize(const char *name, size_t block_size, size_t wb_size, size_t lru_size, short bloom_size)
{

    struct _leveldb_stuff *ldbs = NULL;
    leveldb_cache_t *cache;
    leveldb_filterpolicy_t *policy;
    char* err = NULL;

    ldbs = malloc(sizeof(struct _leveldb_stuff));
    memset(ldbs, 0, sizeof(struct _leveldb_stuff));

    ldbs->options = leveldb_options_create();

    snprintf(ldbs->dbname, sizeof(ldbs->dbname), "%s", name);

    cache = leveldb_cache_create_lru(lru_size);
    policy = leveldb_filterpolicy_create_bloom(bloom_size);

    leveldb_options_set_filter_policy(ldbs->options, policy);
    leveldb_options_set_cache(ldbs->options, cache);
    leveldb_options_set_block_size(ldbs->options, block_size);
    leveldb_options_set_write_buffer_size(ldbs->options, wb_size);
#if defined(OPEN_COMPRESSION)
    leveldb_options_set_compression(ldbs->options, leveldb_snappy_compression);
#else
    leveldb_options_set_compression(ldbs->options, leveldb_no_compression);
#endif
    /* R */
    ldbs->roptions = leveldb_readoptions_create();
    leveldb_readoptions_set_verify_checksums(ldbs->roptions, 1);
    leveldb_readoptions_set_fill_cache(ldbs->roptions, 1);/* set 1 is faster */

    /* W */
    ldbs->woptions = leveldb_writeoptions_create();
#ifdef SYNC_PUT
    leveldb_writeoptions_set_sync(ldbs->woptions, 1);
#else
    leveldb_writeoptions_set_sync(ldbs->woptions, 0);
#endif

    /* B */
    ldbs->wbatch = leveldb_writebatch_create();

    leveldb_options_set_create_if_missing(ldbs->options, 1);
    ldbs->db = leveldb_open(ldbs->options, ldbs->dbname, &err);

    if (err) {
        fprintf(stderr, "%s", err);
        leveldb_free(err);
        err = NULL;
        free(ldbs);
        return NULL ;
    }

    return ldbs;
}

/*
 * Close the ldb.
 */
void ldb_close(struct _leveldb_stuff *ldbs)
{
    leveldb_close(ldbs->db);
    leveldb_options_destroy(ldbs->options);
    leveldb_readoptions_destroy(ldbs->roptions);
    leveldb_writeoptions_destroy(ldbs->woptions);
    leveldb_writebatch_destroy(ldbs->wbatch);
}

/*
 * Destroy the ldb.
 */
void ldb_destroy(struct _leveldb_stuff *ldbs)
{
    char* err = NULL;
    leveldb_close(ldbs->db);
    leveldb_destroy_db(ldbs->options, ldbs->dbname, &err);
    if (err) {
        fprintf(stderr, "%s\n", err);
        leveldb_free(err);
        err = NULL;
    }
    leveldb_options_destroy(ldbs->options);
    leveldb_readoptions_destroy(ldbs->roptions);
    leveldb_writeoptions_destroy(ldbs->woptions);
    leveldb_writebatch_destroy(ldbs->wbatch);
    free(ldbs);
}

char *ldb_get(struct _leveldb_stuff *ldbs, const char *key, size_t klen, int *vlen)
{
    char *err = NULL;
    char *val = NULL;
    val = leveldb_get(ldbs->db, ldbs->roptions, key, klen, (size_t *) vlen, &err);
    if (err) {
        fprintf(stderr, "%s\n", err);
        leveldb_free(err);
        err = NULL;
        *vlen = -1;
        return NULL ;
    } else {
        return val;
    }
}

inline int ldb_put(struct _leveldb_stuff *ldbs, const char *key, size_t klen, const char *value, size_t vlen)
{
    char *err = NULL;
    leveldb_put(ldbs->db, ldbs->woptions, key, klen, value, vlen, &err);
    if (err) {
        fprintf(stderr, "%s\n", err);
        leveldb_free(err);
        err = NULL;
        return -1;
    } else {
        return 0;
    }
}

int ldb_pull(struct _leveldb_stuff *ldbs, const char *key, size_t klen, char *value, size_t vlen)
{
	size_t len = 0;
	char *err = NULL;
	char *val = NULL;
	val = leveldb_get(ldbs->db, ldbs->roptions, key, klen, &len, &err);
	if (err) {
		fprintf(stderr, "%s\n", err);
		leveldb_free(err);
		err = NULL;
		return -1;
	} else {
		if (!val){
			return -1;
		}
		int ok = (vlen == len) ? 0 : -1;
		if (!ok){
			memcpy(value, val, vlen);
		}
		leveldb_free(val);
		return ok;
	}
}

inline int ldb_delete(struct _leveldb_stuff *ldbs, const char *key, size_t klen)
{
    char* err = NULL;
    char *val = NULL;
    size_t vlen = 0;
    val = leveldb_get(ldbs->db, ldbs->roptions, key, klen, (size_t *) &vlen, &err);
    if (err) {
        fprintf(stderr, "%s\n", err);
        leveldb_free(err);
        err = NULL;
        return -1;
    }
    leveldb_free(val);

    /* if not found, then return 0. */
    if (vlen == 0) {
        return 0;
    }

    /* if found, delete it, then return 1. */
    leveldb_delete(ldbs->db, ldbs->woptions, key, klen, &err);
    if (err) {
        fprintf(stderr, "%s\n", err);
        leveldb_free(err);
        err = NULL;
        return -1;
    }

    return 1;
}

inline int ldb_batch_put(struct _leveldb_stuff *ldbs, const char *key, size_t klen, const char *value, size_t vlen)
{
    leveldb_writebatch_put(ldbs->wbatch, key, klen, value, vlen);
    return 0;
}

inline int ldb_batch_delete(struct _leveldb_stuff *ldbs, const char *key, size_t klen)
{
    leveldb_writebatch_delete(ldbs->wbatch, key, klen);
    return 0;
}

inline int ldb_batch_commit(struct _leveldb_stuff *ldbs)
{
    char *err = NULL;
    leveldb_write(ldbs->db, ldbs->woptions, ldbs->wbatch, &err);
    leveldb_writebatch_clear(ldbs->wbatch);
    if (err) {
        fprintf(stderr, "%s\n", err);
        leveldb_free(err);
        err = NULL;
        return -1;
    } else {
        return 0;
    }
}

inline int ldb_exists(struct _leveldb_stuff *ldbs, const char *key, size_t klen)
{
    char *err = NULL;
    char *val = NULL;
    size_t vlen = 0;
    val = leveldb_get(ldbs->db, ldbs->roptions, key, klen, (size_t *) &vlen, &err);
    if (err) {
        fprintf(stderr, "%s\n", err);
        leveldb_free(err);
        err = NULL;
        return -1;
    }
    leveldb_free(val);

    return vlen ? 1 : 0;
}

inline void ldb_compact(struct _leveldb_stuff *ldbs)
{
    leveldb_compact_range(ldbs->db, NULL, 0, NULL, 0);
} 
