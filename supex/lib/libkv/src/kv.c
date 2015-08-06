#include "libkv.h"
#include "kv_inner.h"
#include <sys/wait.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/uio.h>
#include <float.h>
#include <math.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <locale.h>



struct redisServer server;
static int kv_already_init;
pthread_mutex_t kv_calling_lock = PTHREAD_MUTEX_INITIALIZER;


/* 
 *
 * Every entry is composed of the following fields:
 *
 * name: a string representing the command name.
 * function: pointer to the C function implementing the command.
 * arity: number of arguments, it is possible to use -N to say >= N
 * sflags: command flags as string. See below for a table of flags.
 * flags: flags as bitmask. Computed by Redis using the 'sflags' field.
 * get_keys_proc: an optional function to get key arguments from a command.
 *                This is only used when the following three fields are not
 *                enough to specify what arguments are keys.
 * first_key_index: first argument that is a key
 * last_key_index: last argument that is a key
 * key_step: step to get all the keys from first to last argument. For instance
 *           in MSET the step is two since arguments are key,val,key,val,...
 * microseconds: microseconds of total execution time for this command.
 * calls: total number of calls of this command.
 *
 * The flags, microseconds and calls fields are computed by Redis and should
 * always be set to zero.
 *
 * Command flags are expressed using strings where every character represents
 * a flag. Later the populateCommandTable() function will take care of
 * populating the real 'flags' field using this characters.
 *
 * This is the meaning of the flags:
 *
 * w: write command (may modify the key space).
 * r: read command  (will never modify the key space).
 * m: may increase memory usage once called. Don't allow if out of memory. 
 * a: admin command, like SAVE or SHUTDOWN.
 * p: Pub/Sub related command.
 * f: force replication of this command, regardless of server.dirty.
 * s: command not allowed in scripts.
 * R: random command. Command is not deterministic, that is, the same command
 *    with the same arguments, with the same key space, may have different
 *    results. For instance SPOP and RANDOMKEY are two random commands.
 * S: Sort command output array if called from script, so that the output
 *    is deterministic.
 * l: Allow command while loading the database.
 * t: Allow command while a slave has stale data but is not allowed to
 *    server this data. Normally no command is accepted in this condition
 *    but just a few.
 * M: Do not automatically propagate the command on MONITOR.
 * k: Perform an implicit ASKING for this command, so the command will be
 *    accepted in cluster mode if the slot is marked as 'importing'.
 * F: Fast command: O(1) or O(log(N)) command that should never delay
 *    its execution as long as the kernel scheduler is giving us time.
 *    Note that commands that may trigger a DEL as a side effect (like SET)
 *    are not fast commands.
 */
struct redisCommand redisCommandTable[] = {
        {"set",setCommand,3,"wm",0,NULL,1,1,1,0,0},
        {"get",getCommand,2,"rF",0,NULL,1,1,1,0,0},
        {"del",delCommand,-2,"w",0,NULL,1,-1,1,0,0},
        {"incr",incrCommand,2,"wmF",0,NULL,1,1,1,0,0},
        {"incrby",incrbyCommand,3,"wmF",0,NULL,1,1,1,0,0},
        {"decr",decrCommand,2,"wmF",0,NULL,1,1,1,0,0},
        {"decrby",decrbyCommand,3,"wmF",0,NULL,1,1,1,0,0},
        {"dbsize",dbsizeCommand,1,"rF",0,NULL,0,0,0,0,0},
        {"flushdb",flushdbCommand,1,"w",0,NULL,0,0,0,0,0},
        {"lpush",lpushCommand,-3,"wmF",0,NULL,1,1,1,0,0},
        {"lrange",lrangeCommand,4,"r",0,NULL,1,1,1,0,0},
        {"exists",existsCommand,2,"rF",0,NULL,1,1,1,0,0},
        {"sadd",saddCommand,-3,"wmF",0,NULL,1,1,1,0,0},
        {"smembers",smembersCommand,2,"rS",0,NULL,1,1,1,0,0},
        {"expire",expireCommand,3,"wF",0,NULL,1,1,1,0,0},
        {"expireat",expireatCommand,3,"wF",0,NULL,1,1,1,0,0},
        {"pexpire",pexpireCommand,3,"wF",0,NULL,1,1,1,0,0},
        {"pexpireat",pexpireatCommand,3,"wF",0,NULL,1,1,1,0,0}
//        {"zadd",zaddCommand,-4,"wmF",0,NULL,1,1,1,0,0},
//        {"zrange",zrangeCommand,-4,"r",0,NULL,1,1,1,0,0},
//        {"hset",hsetCommand,4,"wmF",0,NULL,1,1,1,0,0},
//        {"hget",hgetCommand,3,"rF",0,NULL,1,1,1,0,0}
/*        {"scard",scardCommand,2,"rF",0,NULL,1,1,1,0,0},
        {"select",selectCommand,2,"rlF",0,NULL,0,0,0,0,0},
        {"sismember",sismemberCommand,3,"rF",0,NULL,1,1,1,0,0},
        {"type",typeCommand,2,"rF",0,NULL,1,1,1,0,0},
        {"zcard",zcardCommand,2,"rF",0,NULL,1,1,1,0,0}
*/
};

unsigned int dictSdsCaseHash(const void *key);
int dictSdsKeyCaseCompare(void *privdata, const void *key1, const void *key2);
void dictSdsDestructor(void *privdata, void *val);
unsigned int dictSdsHash(const void *key);
int dictSdsKeyCompare(void *privdata, const void *key1, const void *key2);
void dictSdsDestructor(void *privdata, void *val);
void dictRedisObjectDestructor(void *privdata, void *val);


dictType commandTableDictType = {
        dictSdsCaseHash,           /* hash function */
        NULL,                      /* key dup */
        NULL,                      /* val dup */
        dictSdsKeyCaseCompare,     /* key compare */
        dictSdsDestructor,         /* key destructor */
        NULL                       /* val destructor */
};

dictType dbDictType = {
        dictSdsHash,
        NULL,
        NULL,
        dictSdsKeyCompare,
        dictSdsDestructor,
        dictRedisObjectDestructor
};


static void populateCommandTable()
{
        int j;
        int numcommands = sizeof(redisCommandTable)/sizeof(struct redisCommand);

        for (j = 0; j < numcommands; j++) {
                struct redisCommand *c = redisCommandTable+j;
                char *f = c->sflags;

                while(*f != '\0') {
                        switch(*f) {
                        case 'w': c->flags |= REDIS_CMD_WRITE; break;
                        case 'r': c->flags |= REDIS_CMD_READONLY; break;
                        case 'm': c->flags |= REDIS_CMD_DENYOOM; break;
                        case 'a': c->flags |= REDIS_CMD_ADMIN; break;
                        case 'p': c->flags |= REDIS_CMD_PUBSUB; break;
                        case 's': c->flags |= REDIS_CMD_NOSCRIPT; break;
                        case 'R': c->flags |= REDIS_CMD_RANDOM; break;
                        case 'S': c->flags |= REDIS_CMD_SORT_FOR_SCRIPT; break;
                        case 'l': c->flags |= REDIS_CMD_LOADING; break;
                        case 't': c->flags |= REDIS_CMD_STALE; break;
                        case 'M': c->flags |= REDIS_CMD_SKIP_MONITOR; break;
                        case 'k': c->flags |= REDIS_CMD_ASKING; break;
                        case 'F': c->flags |= REDIS_CMD_FAST; break;
                    
                        default:logicError("Unsupported command flag");
                        }
                        f++;
                }
                logicErrorExpr(dictAdd(server.commands, sdsnew(c->name), c) == DICT_OK, "Create command table failed");
        }  
}

void createSharedObjects(void)
{
        int i;
    
        shared.ok = createObject(REDIS_STRING,sdsnew("OK"));

        for (i = 0; i < REDIS_SHARED_INTEGERS; i++) {
                shared.integers[i] = createObject(REDIS_STRING, (void*)(long)i);
                shared.integers[i]->encoding = REDIS_ENCODING_INT;
        }
}

struct sharedObjectsStruct shared;

/* Global vars that are actually used as constants. The following double
 * values are used for double on-disk serialization, and are initialized
 * at runtime to avoid strange compiler optimizations. */

double R_Zero, R_PosInf, R_NegInf, R_Nan;

struct evictionPoolEntry *evictionPoolAlloc(void);

/* This is a hash table type that uses the SDS dynamic strings library as
 * keys and redis objects as values (objects can hold SDS strings,
 * lists, sets). */

void dictVanillaFree(void *privdata, void *val)
{
        KV_NOTUSED(privdata);
        KV_NOTUSED(val);
}

void dictListDestructor(void *privdata, void *val)
{
        KV_NOTUSED(privdata);
        KV_NOTUSED(val);
}

int dictSdsKeyCompare(void *privdata, const void *key1,
                      const void *key2)
{
        int l1,l2;
        DICT_NOTUSED(privdata);

        l1 = sdslen((sds)key1);
        l2 = sdslen((sds)key2);
        if (l1 != l2) return 0;
        return memcmp(key1, key2, l1) == 0;
}

/* A case insensitive version used for the command lookup table and other
 * places where case insensitive non binary-safe comparison is needed. */
int dictSdsKeyCaseCompare(void *privdata, const void *key1,
                          const void *key2)
{
        DICT_NOTUSED(privdata);

        return strcasecmp(key1, key2) == 0;
}

void dictRedisObjectDestructor(void *privdata, void *val)
{
        DICT_NOTUSED(privdata);

        if (val == NULL) return; /* Values of swapped out keys as set to NULL */
        decrRefCount(val);
}

void dictSdsDestructor(void *privdata, void *val)
{
        DICT_NOTUSED(privdata);
        
        sdsfree(val);
}

int dictObjKeyCompare(void *privdata, const void *key1,
                      const void *key2)
{
        KV_NOTUSED(privdata);
        KV_NOTUSED(key1);
        KV_NOTUSED(key2);
        return 0; 
}

unsigned int dictObjHash(const void *key)
{
        KV_NOTUSED(key);
        return 0;
}

unsigned int dictSdsHash(const void *key)
{
        return dictGenHashFunction((unsigned char*)key, sdslen((char*)key));
}

unsigned int dictSdsCaseHash(const void *key)
{
        return dictGenCaseHashFunction((unsigned char*)key, sdslen((char*)key));
}

int dictEncObjKeyCompare(void *privdata, const void *key1,
                         const void *key2)
{
        robj *o1 = (robj*) key1, *o2 = (robj*) key2;
        int cmp;

        if (o1->encoding == REDIS_ENCODING_INT &&
            o2->encoding == REDIS_ENCODING_INT)
                return o1->ptr == o2->ptr;

        o1 = getDecodedObject(o1);
        o2 = getDecodedObject(o2);
        cmp = dictSdsKeyCompare(privdata,o1->ptr,o2->ptr);
        decrRefCount(o1);
        decrRefCount(o2);
        return cmp;
}

unsigned int dictEncObjHash(const void *key)
{
    robj *o = (robj*) key;

    if (sdsEncodedObject(o)) {
        return dictGenHashFunction(o->ptr, sdslen((sds)o->ptr));
    } else {
        if (o->encoding == REDIS_ENCODING_INT) {
            char buf[32];
            int len;

            len = ll2string(buf,32,(long)o->ptr);
            return dictGenHashFunction((unsigned char*)buf, len);
        } else {
            unsigned int hash;

            o = getDecodedObject(o);
            hash = dictGenHashFunction(o->ptr, sdslen((sds)o->ptr));
            decrRefCount(o);
            return hash;
        }
    }
}


/* Sets type hash table */
dictType setDictType = {
    dictEncObjHash,            /* hash function */
    NULL,                      /* key dup */
    NULL,                      /* val dup */
    dictEncObjKeyCompare,      /* key compare */
    dictRedisObjectDestructor, /* key destructor */
    NULL                       /* val destructor */
};

/* Sorted sets hash (note: a skiplist is used in addition to the hash table) */
dictType zsetDictType = {
    dictEncObjHash,            /* hash function */
    NULL,                      /* key dup */
    NULL,                      /* val dup */
    dictEncObjKeyCompare,      /* key compare */
    dictRedisObjectDestructor, /* key destructor */
    NULL                       /* val destructor */
};


/* server.lua_scripts sha (as sds string) -> scripts (as robj) cache. */
dictType shaScriptObjectDictType = {
        dictSdsCaseHash,            /* hash function */
        NULL,                       /* key dup */
        NULL,                       /* val dup */
        dictSdsKeyCaseCompare,      /* key compare */
        dictSdsDestructor,          /* key destructor */
        dictRedisObjectDestructor   /* val destructor */
};

/* Db->expires */
dictType keyptrDictType = {
        dictSdsHash,               /* hash function */
        NULL,                      /* key dup */
        NULL,                      /* val dup */
        dictSdsKeyCompare,         /* key compare */
        NULL,                      /* key destructor */
        NULL                       /* val destructor */
};



/* Hash type hash table (note that small hashes are represented with ziplists) */
dictType hashDictType = {
        dictEncObjHash,             /* hash function */
        NULL,                       /* key dup */
        NULL,                       /* val dup */
        dictEncObjKeyCompare,       /* key compare */
        dictRedisObjectDestructor,  /* key destructor */
        dictRedisObjectDestructor   /* val destructor */
};

/* Keylist hash table type has unencoded redis objects as keys and
 * lists as values. It's used for blocking operations (BLPOP) and to
 * map swapped keys to a list of clients waiting for this keys to be loaded. */
dictType keylistDictType = {
        dictObjHash,                /* hash function */
        NULL,                       /* key dup */
        NULL,                       /* val dup */
        dictObjKeyCompare,          /* key compare */
        dictRedisObjectDestructor,  /* key destructor */
        dictListDestructor          /* val destructor */
};

/* Cluster nodes hash table, mapping nodes addresses 1.2.3.4:6379 to
 * clusterNode structures. */
dictType clusterNodesDictType = {
        dictSdsHash,                /* hash function */
        NULL,                       /* key dup */
        NULL,                       /* val dup */
        dictSdsKeyCompare,          /* key compare */
        dictSdsDestructor,          /* key destructor */
        NULL                        /* val destructor */
};

/* Cluster re-addition blacklist. This maps node IDs to the time
 * we can re-add this node. The goal is to avoid readding a removed
 * node for some time. */
dictType clusterNodesBlackListDictType = {
        dictSdsCaseHash,            /* hash function */
        NULL,                       /* key dup */
        NULL,                       /* val dup */
        dictSdsKeyCaseCompare,      /* key compare */
        dictSdsDestructor,          /* key destructor */
        NULL                        /* val destructor */
};

/* Migrate cache dict type. */
dictType migrateCacheDictType = {
        dictSdsHash,                /* hash function */
        NULL,                       /* key dup */
        NULL,                       /* val dup */
        dictSdsKeyCompare,          /* key compare */
        dictSdsDestructor,          /* key destructor */
        NULL                        /* val destructor */
};

/* Replication cached script dict (server.repl_scriptcache_dict).
 * Keys are sds SHA1 strings, while values are not used at all in the current
 * implementation. */
dictType replScriptCacheDictType = {
        dictSdsCaseHash,            /* hash function */
        NULL,                       /* key dup */
        NULL,                       /* val dup */
        dictSdsKeyCaseCompare,      /* key compare */
        dictSdsDestructor,          /* key destructor */
        NULL                        /* val destructor */
};

int htNeedsResize(dict *dict)
{
        KV_NOTUSED(dict);
        return 0;
}

/* If the percentage of used slots in the HT reaches REDIS_HT_MINFILL
 * we resize the hash table to save memory */
void tryResizeHashTables(int dbid)
{
        KV_NOTUSED(dbid);
}

/* Our hash table implementation performs rehashing incrementally while
 * we write/read from the hash table. Still if the server is idle, the hash
 * table will use two tables for a long time. So we try to use 1 millisecond
 * of CPU time at every call of this function to perform some rehahsing.
 *
 * The function returns 1 if some rehashing was performed, otherwise 0
 * is returned. */
int incrementallyRehash(int dbid)
{
        KV_NOTUSED(dbid);
        return 0;
}

/* This function is called once a background process of some kind terminates,
 * as we want to avoid resizing the hash tables when there is a child in order
 * to play well with copy-on-write (otherwise when a resize happens lots of
 * memory pages are copied). The goal of this function is to update the ability
 * for dict.c to resize the hash tables accordingly to the fact we have o not
 * running childs. */
void updateDictResizePolicy(void) {
}


int activeExpireCycleTryExpire(redisDb *db, dictEntry *de, long long now)
{
        KV_NOTUSED(db);
        KV_NOTUSED(de);
        KV_NOTUSED(now);
        return REDIS_ERR;
}


void activeExpireCycle(int type)
{
        KV_NOTUSED(type);
}

unsigned int getLRUClock(void)
{
        return 0;
}

/* Add a sample to the operations per second array of samples. */
void trackInstantaneousMetric(int metric, long long current_reading)
{
        KV_NOTUSED(metric);
        KV_NOTUSED(current_reading);
}

/* Return the mean of all the samples. */
long long getInstantaneousMetric(int metric)
{
        KV_NOTUSED(metric);
        return 0;
}

/* Check for timeouts. Returns non-zero if the client was terminated */
/*
int clientsCronHandleTimeout(redisClient *c)
{
        KV_NOTUSED(c);
        return 0;
}
*/

/* The client query buffer is an sds.c string that can end with a lot of
 * free space not used, this function reclaims space if needed.
 *
 * The function always returns 0 as it never terminates the client. */
 /*
int clientsCronResizeQueryBuffer(redisClient *c)
{
        KV_NOTUSED(c);
        return 0;
}
 */

/* This function handles 'background' operations we are required to do
 * incrementally in Redis databases, such as active key expiring, resizing,
 * rehashing. */
void databasesCron(void) {

}

/* We take a cached value of the unix time in the global state because with
 * virtual memory and aging there is to store the current time in objects at
 * every object access, and accuracy is not needed. To access a global var is
 * a lot faster than calling time(NULL) */
void updateCachedTime(void) {
}

void resetCommandTableStats(void) {

}


void redisOpArrayInit(redisOpArray *oa)
{
        KV_NOTUSED(oa);
}

int redisOpArrayAppend(redisOpArray *oa, struct redisCommand *cmd, int dbid,
                       robj **argv, int argc, int target)
{
        KV_NOTUSED(oa);
        KV_NOTUSED(cmd);
        KV_NOTUSED(dbid);
        KV_NOTUSED(argv);
        KV_NOTUSED(argc);
        KV_NOTUSED(target);

        return 0;
}

void redisOpArrayFree(redisOpArray *oa)
{
        KV_NOTUSED(oa);
}

/* Propagate the specified command (in the context of the specified database id)
 * to AOF and Slaves.
 *
 * flags are an xor between:
 * + REDIS_PROPAGATE_NONE (no propagation of command at all)
 * + REDIS_PROPAGATE_AOF (propagate into the AOF file if is enabled)
 * + REDIS_PROPAGATE_REPL (propagate into the replication link)
 */
void propagate(struct redisCommand *cmd, int dbid, robj **argv, int argc,
               int flags)
{
        KV_NOTUSED(cmd);
        KV_NOTUSED(dbid);
        KV_NOTUSED(argv);
        KV_NOTUSED(argc);
        KV_NOTUSED(flags);
}

/* Used inside commands to schedule the propagation of additional commands
 * after the current command is propagated to AOF / Replication. */
void alsoPropagate(struct redisCommand *cmd, int dbid, robj **argv, int argc,
                   int target)
{
        KV_NOTUSED(cmd);
        KV_NOTUSED(dbid);
        KV_NOTUSED(argv);
        KV_NOTUSED(argc);
        KV_NOTUSED(target);
}

/* It is possible to call the function forceCommandPropagation() inside a
 * Redis command implementation in order to to force the propagation of a
 * specific command execution into AOF / Replication. */
/*
void forceCommandPropagation(redisClient *c, int flags)
{
        KV_NOTUSED(c);
        KV_NOTUSED(flags);
}
*/

int time_independent_strcmp(char *a, char *b)
{
        KV_NOTUSED(a);
        KV_NOTUSED(b);
        return 0;
}

/* Create the string returned by the INFO command. This is decoupled
 * by the INFO command itself as we need to report the same information
 * on memory corruption problems. */
sds genRedisInfoString(char *section)
{
        KV_NOTUSED(section);
        return NULL;
}

/* Create a new eviction pool. */
struct evictionPoolEntry *evictionPoolAlloc(void)
{
        return NULL;
}

#define EVICTION_SAMPLES_ARRAY_SIZE 16
void evictionPoolPopulate(dict *sampledict, dict *keydict, struct evictionPoolEntry *pool)
{
        KV_NOTUSED(sampledict);
        KV_NOTUSED(keydict);
        KV_NOTUSED(pool);
}

long long ustime()
{
        struct timeval cur;
        gettimeofday(&cur, NULL);
        return cur.tv_sec*1000000 + cur.tv_usec;
}

long long mstime()
{
        return ustime()/1000;
}

struct redisCommand *lookupCommand(sds name)
{
        return dictFetchValue(server.commands, name);
}



/**
 * @ingroup group_error_code
 * @{
 */
#define ERR_TABLE_STEP 5
static int err_table_size = 0;
static const char **err_table = NULL;



static void err_table_add(int errnum, const char* desc)
{
        if (err_table_size == 0) {
                err_table = zmalloc(sizeof(char*)*ERR_TABLE_STEP);
                err_table_size = ERR_TABLE_STEP;
        }

        if (errnum >= err_table_size) {
                int incr = err_table_size;
                while(errnum >= incr) {
                        incr += ERR_TABLE_STEP;
                }
                err_table = zrealloc(err_table, sizeof(char*)*incr);
                err_table_size = incr;
        }

        err_table[errnum] = desc;
}

const char* err_table_get(int errnum)
{
        return err_table[errnum];
}

void err_table_create()
{
        err_table_add(ERR_NONE, "no error");
        err_table_add(ERR_TYPE, "-ERR wrong type");
        err_table_add(ERR_NO_KEY, "-ERR no such key");
        err_table_add(ERR_SYNTAX, "-ERR syntax error");
        err_table_add(ERR_SAME_OBJECT, "-ERR source and destination objects are the same");
        err_table_add(ERR_OUT_OF_RANGE, "-ERR index out of range");
        err_table_add(ERR_NIL, "-ERR nil");
        err_table_add(ERR_NOT_INIT, "-ERR not init");
        err_table_add(ERR_ARGUMENTS, "-ERR wrong number of arguments");
        err_table_add(ERR_PROTOCOL, "-ERR protocol error");
        err_table_add(ERR_VALUE, "-ERR value invalid");
        err_table_add(ERR_DB_INDEX, "-ERR invalid DB index");
        err_table_add(ERR_CMD, "-ERR unknown command");
}

void err_table_release()
{
        zfree(err_table);
        err_table_size = 0;
}
/**
 * @}
 */
   



void timer_expire()
{

}

void timer_cron()
{
        server.timer = kv_timer_create(KV_DEFAULT_TIMER_INTERVAL);
        kv_timer_add(server.timer, 1000, timer_expire);
        kv_timer_start(server.timer);
}


answer_t* answer_create()
{
        answer_t* r = zmalloc(sizeof(*r));
        if (r == NULL) return r;

        if (kv_already_init) {
                r->errnum = ERR_NONE;
                r->err = err_table_get(r->errnum);
        } else {
                r->errnum = ERR_NOT_INIT;
                r->err = "libkv not init";
        }
        r->count = 0;
        r->head = r->tail = NULL;

        return r;
}

void answer_release(answer_t *a)
{
        answer_value_t *cur, *del;
        if (a == NULL) return;

        cur = a->head;
        while(cur) {
                del = cur;
                cur = cur->next;
                zfree(del->ptr);
                zfree(del);
        }
    
        zfree(a);
}

unsigned long answer_length(answer_t *a)
{
        return a == NULL ? 0 : a->count;
}

answer_value_t *answer_value_create(const void *ptr, unsigned long ptrlen)
{
        answer_value_t* value;
        if (!ptr || ptrlen == 0) return NULL;

        value = zmalloc(sizeof(answer_value_t));
        value->ptr = (char*)ptr;
        value->ptrlen = ptrlen;
        value->prev = value->next = NULL;

        return value;
}

void answer_add_value_tail(answer_t *a, const void *ptr, unsigned long ptrlen)
{
        answer_value_t *value;
        if (!a || !ptr || ptrlen == 0) return;

        value = answer_value_create(ptr, ptrlen);
        if (a->head == NULL && a->tail == NULL) {
                a->head = a->tail = value;
        } else {
                a->tail->next = value;
                value->prev = a->tail;
                a->tail = value;
        }
        a->count++;
}

void answer_add_value_head(answer_t *a, const void *ptr, unsigned long ptrlen)
{
        answer_value_t *value;
        if (!a || !ptr || ptrlen == 0) return;

        value = answer_value_create(ptr, ptrlen);
        if (a->head == NULL && a->tail == NULL) {
                a->head = a->tail = value;
        } else {
                a->head->prev = value;
                value->next = a->head;
                a->head = value;
        }
        a->count++;
}

answer_iter_t *answer_get_iter(answer_t *a, int direction)
{
        if (!a) return NULL;
    
        answer_iter_t *iter = zmalloc(sizeof(answer_iter_t));
        iter->direction = direction;
        if (iter->direction == ANSWER_HEAD) {
                iter->next = a->head;
        } else {
                iter->next = a->tail;
        }
        return iter;
}

void answer_rewind_iter(answer_t *a, answer_iter_t *iter)
{
        if (iter->direction == ANSWER_HEAD) {
                iter->next = a->head;
        } else {
                iter->next = a->tail;
        }
}

void answer_release_iter(answer_iter_t *iter)
{
        if (iter)
                zfree(iter);
}

answer_value_t *answer_next(answer_iter_t *iter)
{
        answer_value_t *cur = iter->next;

        if (!cur) return NULL;
        if (iter->direction == ANSWER_HEAD) {
                iter->next = cur->next;
        } else {
                iter->next = cur->prev;
        }

        return cur;
}

answer_value_t *answer_first_value(answer_t *ans)
{
        return ans == NULL ? NULL : ans->head;
}

answer_value_t *answer_last_value(answer_t *ans)
{
        return ans == NULL ? NULL : ans->tail;
}

char* answer_value_to_string(answer_value_t *value)
{
        if (value == NULL || value->ptr == NULL) return NULL;

        value->ptr = zrealloc(value->ptr, value->ptrlen + 1);
        ((char*)value->ptr)[value->ptrlen] = '\0';
        value->ptrlen += 1;
        return (char*)value->ptr;
}

/**
 * create caller instance for every call.
 */
caller_t* caller_create()
{
        caller_t* c = zmalloc(sizeof(*c));
        if (!c) return NULL;

        c->argc = 0;
        c->argv = NULL;
        c->cmd = NULL;
        selectDb(c, server.dbindex);
        c->proto_text = sdsempty();
        c->errnum = ERR_NONE;
        c->err = err_table_get(c->errnum);
        c->result = NULL;
        
        return c;
}

void caller_free(caller_t* c)
{
        int i;

        for (i = 0; i < c->argc; i++) {
                decrRefCount(c->argv[i]);
        }
        zfree(c->argv);
        sdsfree(c->proto_text);
        caller_free_result(c);
        zfree(c);
}

void caller_free_result(caller_t* c)
{
        if (c->result != NULL)
                listRelease(c->result);
}

void caller_set_err(caller_t* c, int errnum)
{
        c->errnum = errnum;
        c->err = err_table[errnum]; 
}

void caller_add_result(caller_t *c, robj *o)
{
        if (c == NULL || o == NULL) return;
    
        if (c->result == NULL) {
                c->result = listCreate();
                listSetFreeMethod(c->result, decrRefCountVoid); 
        }
        listAddNodeTail(c->result, o);
}

void caller_add_result_long(caller_t *c, long value)
{
        robj *result;
        result = createObject(REDIS_STRING, (void*)value);
        result->encoding = REDIS_ENCODING_INT;
        caller_add_result(c, result);
}

void caller_add_result_str(caller_t *c, const char* str)
{
        robj *result;

        if (!str || !str[0]) return;
        result = createStringObject((char*)str, strlen(str));
        caller_add_result(c, result);
}

int caller_start(caller_t* c)
{
        if (processProto(c) == KV_ERR) return KV_ERR;
        c->cmd = lookupCommand(c->argv[0]->ptr);
        if (!c->cmd) {
                caller_set_err(c, ERR_CMD);
                return KV_ERR;
        }
        
        if ((c->cmd->arity > 0 && c->cmd->arity != c->argc) ||
            (c->argc < -c->cmd->arity)) {
                caller_set_err(c, ERR_ARGUMENTS);
                return KV_ERR;
        }
    
        c->cmd->proc(c);
        return ERR_NONE;
}



void initDefaultConfig()
{
        int i;
    
        struct timeval tv;
        server.timer = NULL;
        setlocale(LC_COLLATE,"");
        zmalloc_enable_thread_safeness();
        srand(time(NULL)^getpid());
        gettimeofday(&tv,NULL);
        dictSetHashFunctionSeed(tv.tv_sec^tv.tv_usec^getpid());

        server.maxmemory = 0;
        
        server.dbindex = 0;
        server.dbnum = KV_DEFAULT_DBNUM;
        server.commands = dictCreate(&commandTableDictType, NULL);
        populateCommandTable();
        createSharedObjects();

        server.db = zmalloc(sizeof(redisDb)*server.dbnum);
        for (i = 0; i < server.dbnum; i++) {
                server.db[i].dict = dictCreate(&dbDictType, NULL);
                server.db[i].expires = dictCreate(&keyptrDictType, NULL);
                server.db[i].id = i;
        }

        server.list_max_ziplist_entries = REDIS_LIST_MAX_ZIPLIST_ENTRIES;
        server.list_max_ziplist_value = REDIS_LIST_MAX_ZIPLIST_VALUE;
        server.set_max_intset_entries = REDIS_SET_MAX_INTSET_ENTRIES;
        server.zset_max_ziplist_entries = REDIS_ZSET_MAX_ZIPLIST_ENTRIES;
        server.zset_max_ziplist_value = REDIS_ZSET_MAX_ZIPLIST_VALUE;
        server.hash_max_ziplist_entries = REDIS_HASH_MAX_ZIPLIST_ENTRIES;
        server.hash_max_ziplist_value = REDIS_HASH_MAX_ZIPLIST_VALUE;

        server.stat_keyspace_misses = 0;
        server.stat_keyspace_hits = 0;
        server.stat_expiredkeys = 0;
        server.dirty = 0;
}

/* convert cmd words to protocol */
static int cmd_to_proto(const char* cmd, size_t cmdlen, caller_t* c)
{
        list* cmd_words = split_word(cmd, cmdlen, ' ');
        listIter* iter;
        listNode* node;
        char buf[64];

        if (!cmd_words) return KV_ERR;
        if (cmd_words->len == 0) {
                listRelease(cmd_words);
                return KV_ERR;
        }
        sdsclear(c->proto_text);
        snprintf(buf, sizeof(buf), "*%ld\r\n", cmd_words->len);
        c->proto_text = sdscat(c->proto_text, buf);

        iter = listGetIterator(cmd_words, AL_START_HEAD);
        while((node = listNext(iter))) {
                snprintf(buf, sizeof(buf), "$%ld\r\n", sdslen(node->value));
                c->proto_text = sdscat(c->proto_text, buf);
                c->proto_text = sdscat(c->proto_text, node->value);
                c->proto_text = sdscat(c->proto_text, "\r\n");
        }

        listReleaseIterator(iter);
        listRelease(cmd_words);
        return ERR_NONE;
}

static void stringTypeValue(robj *o, answer_t *a)
{
        int bytes;
        char *lbuf;

        logicErrorExpr(o->type == REDIS_STRING, "Never happend!");

        switch(o->encoding) {
        case REDIS_ENCODING_RAW:
        case REDIS_ENCODING_EMBSTR:
                bytes = sdslen(o->ptr);
                lbuf = zmalloc(bytes);
                memcpy(lbuf, o->ptr, bytes);
                answer_add_value_tail(a, lbuf, bytes);
                break;
        case REDIS_ENCODING_INT:
                lbuf = zmalloc(128);
                logicErrorExpr((bytes = ll2string(lbuf, 128, (long long)o->ptr)) != 0, "Convert string to ll error!");
                answer_add_value_tail(a, lbuf, bytes);
                break;
        default:
                printf("=== forget to handle with type belong to StringType\n");
        }
}

#if 0
static void listTypeValue(robj *o, answer_t *a)
{
        logicErrorExpr(o->type == REDIS_LIST, "Never happend!");
        switch(o->encoding) {
        case REDIS_ENCODING_ZIPLIST:
        case REDIS_ENCODING_LINKEDLIST:
                break;
        default:
                printf("=== to handle with type belong to ListType\n");
        }
        
}

static void setTypeValue(robj *o, answer_t *a)
{
        logicError(o->type == REDIS_SET);
        switch(o->encoding) {

        default:
                printf("=== to handle with type belong to SetType\n");
        }
}

static void zsetTypeValue(robj *o, answer_t *a)
{
        logicErrorExpr(o->type == REDIS_ZSET, "Never happend!");

        switch(o->encoding) {
        default:
                printf("=== forget to handle with type belong to ZsetType\n");

        }
}

static void hashTypeValue(robj *o, answer_t *a)
{
        logicErrorExpr(o->type == REDIS_HASH, "Never happend!");
        switch(o->encoding) {
        default:
                printf("=== to handle with type belong to HashType\n");
        }
}
#endif

void caller_to_answer(caller_t* c, answer_t* ans)
{
        list *l;
        listIter *iter;
        listNode *node;
        robj *tmp;

        if (!c || !ans) return;
        ans->errnum = c->errnum;
        ans->err = c->err;
        if (!c->result) return;
        l = c->result;
        iter = listGetIterator(l, 0/*head*/);

        while((node = listNext(iter)) != NULL) {
                tmp = node->value;
                switch(tmp->type) {
                case REDIS_STRING:
                        stringTypeValue(tmp,ans);break;
                case REDIS_LIST:
//                        listTypeValue(tmp,ans);break;
                case REDIS_SET:
//                        setTypeValue(tmp,ans);break;
                case REDIS_ZSET:
//                        zsetTypeValue(tmp,ans);break;
                case REDIS_HASH:
//                        hashTypeValue(tmp,ans);break;
                        logicErrorExpr(0, "Never happend!");
                default:
                        logicError("Unknown object type!");
                }
        }
        listReleaseIterator(iter);
}

void kv_calling_ready()
{
        pthread_mutex_lock(&kv_calling_lock);
}

void kv_calling_done()
{
        pthread_mutex_unlock(&kv_calling_lock);
}

int processProto(caller_t *c)
{
        char* newline = NULL;
        int pos=0, ok;
        long long ll;
        long long linecount = 0;
        int bulklen = -1;

        /* get line count*/
        newline = strchr(c->proto_text, '\r');
        if (newline == NULL) {
                caller_set_err(c, ERR_PROTOCOL);
                return KV_ERR;
        }

        /* Buffer should also contain \n */
        if (newline-(c->proto_text) > ((signed)sdslen(c->proto_text)-2)) {
                caller_set_err(c, ERR_PROTOCOL);
                return KV_ERR;
        }

        ok = string2ll(c->proto_text+1, newline-(c->proto_text+1),&ll);
        if (!ok || ll > 1024*1024) {
                caller_set_err(c, ERR_PROTOCOL);
                return KV_ERR;
        }

        pos = (newline-c->proto_text)+2;
        if (ll <= 0) { // not exists return ?
                sdsrange(c->proto_text, pos, -1);
                return ERR_NONE;
        }
        linecount = ll;
        if (c->argv) zfree(c->argv);
        c->argv = zmalloc(sizeof(robj*)*linecount);

        while(linecount) {
                if (bulklen == -1) {
                        newline = strchr(c->proto_text+pos, '\r');
                        if (newline == NULL) {
                                caller_set_err(c, ERR_PROTOCOL);
                                return KV_ERR;
                        }
      
                        if (newline-(c->proto_text) > ((signed)sdslen(c->proto_text)-2))
                                break;

                        if (c->proto_text[pos] != '$') {
                                caller_set_err(c, ERR_PROTOCOL);
                                return KV_ERR;
                        }

                        ok = string2ll(c->proto_text+pos+1,newline-(c->proto_text+pos+1),&ll);
                        if (!ok || ll < 0 || ll > 512*1024*1024) {
                                caller_set_err(c, ERR_PROTOCOL);
                                return REDIS_ERR;
                        }

                        pos += newline-(c->proto_text+pos)+2;
                        if (ll >= REDIS_MBULK_BIG_ARG) {
                                size_t qblen;
                                sdsrange(c->proto_text,pos,-1);
                                pos = 0;
                                qblen = sdslen(c->proto_text);

                                if (qblen < (size_t)ll+2)
                                        c->proto_text = sdsMakeRoomFor(c->proto_text,ll+2-qblen);
                        }
                        bulklen = ll;
                }

                if (sdslen(c->proto_text)-pos < (unsigned)(bulklen+2)) {
                        break;
                } else {
                        if (pos == 0 &&
                            bulklen >= REDIS_MBULK_BIG_ARG &&
                            (signed) sdslen(c->proto_text) == bulklen+2)
                        {
                                c->argv[c->argc++] = createObject(REDIS_STRING,c->proto_text);
                                sdsIncrLen(c->proto_text,-2); /* remove CRLF */
                                c->proto_text = sdsempty();
                                c->proto_text = sdsMakeRoomFor(c->proto_text,bulklen+2);
                                pos = 0;
                        } else {
                                c->argv[c->argc++] =
                                        createStringObject(c->proto_text+pos,bulklen);
                                pos += bulklen+2;
                        }
                        bulklen = -1;
                        linecount--;
                }
        }

        if (pos) sdsrange(c->proto_text,pos,-1);
        if (linecount == 0) return ERR_NONE;

        caller_set_err(c, ERR_PROTOCOL);
        return KV_ERR;
}

answer_t* kv_executor(const char* cmd, unsigned int len)
{
        caller_t* c = NULL;
        answer_t* ans = NULL;

        kv_calling_ready();

        ans = answer_create();
        c = caller_create();
        if (cmd_to_proto(cmd, len, c) == KV_ERR) {
                ans->errnum = ERR_ARGUMENTS;
                ans->err = err_table[ERR_ARGUMENTS];
                caller_free(c);
                return ans;
        }
        caller_start(c);
        caller_to_answer(c, ans);
        caller_free(c);

        kv_calling_done();
        return ans;
}



int kv_config(const char* param1, va_list ap)
{
        /* if config failed, abort exit */
        KV_NOTUSED(param1);
        KV_NOTUSED(ap);

        return ERR_NONE;
}

int kv_init(const char* param1, ...)
{
        if (kv_already_init)
                return ERR_NONE;

        kv_calling_ready();

        server.loading = 1;
        if (param1 != NULL) {
                va_list ap;
                va_start(ap, param1);
                kv_config(param1, ap);
        }

        err_table_create();
        initDefaultConfig();
        timer_cron();
        server.loading = 0;
        kv_already_init = 1;
        
        kv_calling_done();
        return ERR_NONE;
}

void kv_uninit()
{
        int i;
        
        if (!kv_already_init)
                return;
        
        kv_calling_ready();
        
        kv_already_init = 0;
        kv_timer_stop(server.timer);
        kv_timer_destroy(server.timer);
        dictRelease(server.commands);
        for (i = 0; i < server.dbnum; i++) {
                dictRelease(server.db[i].dict);
                dictRelease(server.db[i].expires);
        }
        zfree(server.db);
        if (shared.ok->refcount > 1)
                shared.ok->refcount = 1;
        decrRefCount(shared.ok);
        for (i = 0; i < REDIS_SHARED_INTEGERS; i++) {
                if (shared.integers[i]->refcount > 1)
                        shared.integers[i]->refcount = 1;
                decrRefCount(shared.integers[i]);    
        }
        err_table_release();

        kv_calling_done();
}

answer_t* kv_ask(const char *cmd, unsigned int cmdlen)
{
        if (!kv_already_init) {
                return answer_create();
        } else if (!cmd || cmdlen == 0) { /* invalid parameter passed */
                answer_t *a = answer_create();
                a->errnum = ERR_ARGUMENTS;
                a->err = err_table_get(ERR_ARGUMENTS);
                return a;
        }
        return kv_executor(cmd, cmdlen);
}

unsigned int kv_get_used_memory()
{
        return zmalloc_used_memory();
}


