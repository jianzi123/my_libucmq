#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "ldb.h"

const size_t BLOCK_SIZE = 32*1024;
const size_t WRITE_BUFFER_SIZE = 64*1024*1024;
const size_t LRU_CACHE_SIZE = 64*1024*1024;
const short BLOOM_SIZE = 10;

int main()
{
    /* open db. */
    printf("open db\n\n");
    struct _leveldb_stuff *ldbs = ldb_initialize("./data", BLOCK_SIZE,
            WRITE_BUFFER_SIZE, LRU_CACHE_SIZE, BLOOM_SIZE);
    assert(ldbs != NULL);

    /* set key value. */
    printf("test: set key value\n");
    int ret = ldb_put(ldbs, "key", strlen("key"), "value", strlen("value"));
    assert(ret == 0);
    printf("OK\n\n");

    /* get key. */
    printf("test: get key\n");
    int vlen;
    char *value = ldb_get(ldbs, "key", strlen("key"), &vlen);
    assert(strncmp(value, "value", vlen) == 0);
    free(value);
    printf("OK\n\n");

    /* del key. */
    printf("test: del key\n");
    ret = ldb_delete(ldbs, "key", strlen("key"));
    assert(ret == 1);
    printf("OK\n\n");

    /* close db.*/
    printf("close db\n");
    ldb_close(ldbs);

    return 0;
}
