#ifndef _KV_H_
#define _KV_H_




#ifndef KV_OK
#define KV_ERR -1
#endif

#include "fmacros.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <pthread.h>

typedef long long mstime_t; /* millisecond time type. */

#include "sds.h"     /* Dynamic safe strings */
#include "dict.h"    /* Hash tables */
#include "adlist.h"  /* Linked lists */
#include "zmalloc.h" /* total memory usage aware version of malloc/free */
#include "ziplist.h" /* Compact list data structure */
#include "intset.h"  /* Compact integer set structure */
#include "util.h"    /* Misc functions useful in many places */
#include "kv_timer.h"



#define REDIS_LRU_BITS 24
#define REDIS_LRU_CLOCK_MAX ((1<<REDIS_LRU_BITS)-1) /* Max value of obj->lru */
#define REDIS_LRU_CLOCK_RESOLUTION 1000 /* LRU clock resolution in ms */
typedef struct redisObject {
        unsigned type:4;
        unsigned encoding:4;
        unsigned lru:REDIS_LRU_BITS; /* lru time (relative to server.lruclock) */
        int refcount;
        void *ptr;
} robj;


/* Error codes */
#define REDIS_OK                0
#define REDIS_ERR               -1

#define REDIS_CONFIGLINE_MAX    1024
#define REDIS_DBCRON_DBS_PER_CALL 16
#define REDIS_MAX_WRITE_PER_EVENT (1024*64)
#define REDIS_SHARED_SELECT_CMDS 10
#define REDIS_SHARED_INTEGERS 10000
#define REDIS_SHARED_BULKHDR_LEN 32
#define REDIS_MAX_LOGMSG_LEN    1024 /* Default maximum length of syslog messages */
#define REDIS_REPL_TIMEOUT 60
#define REDIS_REPL_PING_SLAVE_PERIOD 10
#define REDIS_RUN_ID_SIZE 40
#define REDIS_EOF_MARK_SIZE 40
#define REDIS_DEFAULT_REPL_BACKLOG_SIZE (1024*1024)    /* 1mb */
#define REDIS_DEFAULT_REPL_BACKLOG_TIME_LIMIT (60*60)  /* 1 hour */
#define REDIS_REPL_BACKLOG_MIN_SIZE (1024*16)          /* 16k */
#define REDIS_BGSAVE_RETRY_DELAY 5 /* Wait a few secs before trying again. */
#define REDIS_DEFAULT_PID_FILE "/var/run/redis.pid"
#define REDIS_DEFAULT_SYSLOG_IDENT "redis"
#define REDIS_DEFAULT_CLUSTER_CONFIG_FILE "nodes.conf"
#define REDIS_DEFAULT_UNIX_SOCKET_PERM 0
#define REDIS_DEFAULT_TCP_KEEPALIVE 0
#define REDIS_DEFAULT_LOGFILE ""
#define REDIS_DEFAULT_SYSLOG_ENABLED 0
#define REDIS_DEFAULT_STOP_WRITES_ON_BGSAVE_ERROR 1
#define REDIS_DEFAULT_REPL_DISKLESS_SYNC 0
#define REDIS_DEFAULT_REPL_DISKLESS_SYNC_DELAY 5
#define REDIS_DEFAULT_REPL_DISABLE_TCP_NODELAY 0
#define REDIS_DEFAULT_MAXMEMORY 0
#define REDIS_DEFAULT_MAXMEMORY_SAMPLES 5
#define REDIS_DEFAULT_ACTIVE_REHASHING 1
#define REDIS_PEER_ID_LEN (REDIS_IP_STR_LEN+32) /* Must be enough for ip:port */
#define REDIS_BINDADDR_MAX 16
#define REDIS_MIN_RESERVED_FDS 32
#define REDIS_DEFAULT_LATENCY_MONITOR_THRESHOLD 0
#define KV_DEFAULT_TIMER_INTERVAL 1000 /** microseconds */


/* Instantaneous metrics tracking. */
#define REDIS_METRIC_SAMPLES 16     /* Number of samples per metric. */
#define REDIS_METRIC_COMMAND 0      /* Number of commands executed. */
#define REDIS_METRIC_NET_INPUT 1    /* Bytes read to network .*/
#define REDIS_METRIC_NET_OUTPUT 2   /* Bytes written to network. */
#define REDIS_METRIC_COUNT 3

/* Protocol and I/O related defines */
#define REDIS_MAX_QUERYBUF_LEN  (1024*1024*1024) /* 1GB max query buffer. */
#define REDIS_IOBUF_LEN         (1024*16)  /* Generic I/O buffer size */
#define REDIS_REPLY_CHUNK_BYTES (16*1024) /* 16k output buffer */
#define REDIS_INLINE_MAX_SIZE   (1024*64) /* Max size of inline reads */
#define REDIS_MBULK_BIG_ARG     (1024*32)
#define REDIS_LONGSTR_SIZE      21          /* Bytes needed for long -> str */

/* When configuring the Redis eventloop, we setup it so that the total number
 * of file descriptors we can handle are server.maxclients + RESERVED_FDS + FDSET_INCR
 * that is our safety margin. */
#define REDIS_EVENTLOOP_FDSET_INCR (REDIS_MIN_RESERVED_FDS+96)

/* Hash table parameters */
#define REDIS_HT_MINFILL        10      /* Minimal hash table fill 10% */

/* Command flags. Please check the command table defined in the redis.c file
 * for more information about the meaning of every flag. */
#define REDIS_CMD_WRITE 1                   /* "w" flag */
#define REDIS_CMD_READONLY 2                /* "r" flag */
#define REDIS_CMD_DENYOOM 4                 /* "m" flag */
#define REDIS_CMD_NOT_USED_1 8              /* no longer used flag */
#define REDIS_CMD_ADMIN 16                  /* "a" flag */
#define REDIS_CMD_PUBSUB 32                 /* "p" flag */
#define REDIS_CMD_NOSCRIPT  64              /* "s" flag */
#define REDIS_CMD_RANDOM 128                /* "R" flag */
#define REDIS_CMD_SORT_FOR_SCRIPT 256       /* "S" flag */
#define REDIS_CMD_LOADING 512               /* "l" flag */
#define REDIS_CMD_STALE 1024                /* "t" flag */
#define REDIS_CMD_SKIP_MONITOR 2048         /* "M" flag */
#define REDIS_CMD_ASKING 4096               /* "k" flag */
#define REDIS_CMD_FAST 8192                 /* "F" flag */

/* Object types */
#define REDIS_STRING 0
#define REDIS_LIST 1
#define REDIS_SET 2
#define REDIS_ZSET 3
#define REDIS_HASH 4

/* Objects encoding. Some kind of objects like Strings and Hashes can be
 * internally represented in multiple ways. The 'encoding' field of the object
 * is set to one of this fields for this object. */
#define REDIS_ENCODING_RAW 0     /* Raw representation */
#define REDIS_ENCODING_INT 1     /* Encoded as integer */
#define REDIS_ENCODING_HT 2      /* Encoded as hash table */
#define REDIS_ENCODING_ZIPMAP 3  /* Encoded as zipmap */
#define REDIS_ENCODING_LINKEDLIST 4 /* Encoded as regular linked list */
#define REDIS_ENCODING_ZIPLIST 5 /* Encoded as ziplist */
#define REDIS_ENCODING_INTSET 6  /* Encoded as intset */
#define REDIS_ENCODING_SKIPLIST 7  /* Encoded as skiplist */
#define REDIS_ENCODING_EMBSTR 8  /* Embedded sds string encoding */

/* Client flags */
#define REDIS_SLAVE (1<<0)   /* This client is a slave server */
#define REDIS_MASTER (1<<1)  /* This client is a master server */
#define REDIS_MONITOR (1<<2) /* This client is a slave monitor, see MONITOR */
#define REDIS_MULTI (1<<3)   /* This client is in a MULTI context */
#define REDIS_BLOCKED (1<<4) /* The client is waiting in a blocking operation */
#define REDIS_DIRTY_CAS (1<<5) /* Watched keys modified. EXEC will fail. */
#define REDIS_CLOSE_AFTER_REPLY (1<<6) /* Close after writing entire reply. */
#define REDIS_UNBLOCKED (1<<7) /* This client was unblocked and is stored in
                                  server.unblocked_clients */
#define REDIS_LUA_CLIENT (1<<8) /* This is a non connected client used by Lua */
#define REDIS_ASKING (1<<9)     /* Client issued the ASKING command */
#define REDIS_CLOSE_ASAP (1<<10)/* Close this client ASAP */
#define REDIS_UNIX_SOCKET (1<<11) /* Client connected via Unix domain socket */
#define REDIS_DIRTY_EXEC (1<<12)  /* EXEC will fail for errors while queueing */
#define REDIS_MASTER_FORCE_REPLY (1<<13)  /* Queue replies even if is master */
#define REDIS_FORCE_AOF (1<<14)   /* Force AOF propagation of current cmd. */
#define REDIS_FORCE_REPL (1<<15)  /* Force replication of current cmd. */
#define REDIS_PRE_PSYNC (1<<16)   /* Instance don't understand PSYNC. */
#define REDIS_READONLY (1<<17)    /* Cluster client is in read-only state. */
#define REDIS_PUBSUB (1<<18)      /* Client is in Pub/Sub mode. */

/* Client block type (btype field in client structure)
 * if REDIS_BLOCKED flag is set. */
#define REDIS_BLOCKED_NONE 0    /* Not blocked, no REDIS_BLOCKED flag set. */
#define REDIS_BLOCKED_LIST 1    /* BLPOP & co. */
#define REDIS_BLOCKED_WAIT 2    /* WAIT for synchronous replication. */

/* Client request types */
#define REDIS_REQ_INLINE 1
#define REDIS_REQ_MULTIBULK 2

/* Client classes for client limits, currently used only for
 * the max-client-output-buffer limit implementation. */
#define REDIS_CLIENT_TYPE_NORMAL 0 /* Normal req-reply clients + MONITORs */
#define REDIS_CLIENT_TYPE_SLAVE 1  /* Slaves. */
#define REDIS_CLIENT_TYPE_PUBSUB 2 /* Clients subscribed to PubSub channels. */
#define REDIS_CLIENT_TYPE_COUNT 3

/* Slave replication state - from the point of view of the slave. */
#define REDIS_REPL_NONE 0 /* No active replication */
#define REDIS_REPL_CONNECT 1 /* Must connect to master */
#define REDIS_REPL_CONNECTING 2 /* Connecting to master */
#define REDIS_REPL_RECEIVE_PONG 3 /* Wait for PING reply */
#define REDIS_REPL_TRANSFER 4 /* Receiving .rdb from master */
#define REDIS_REPL_CONNECTED 5 /* Connected to master */

/* Slave replication state - from the point of view of the master.
 * In SEND_BULK and ONLINE state the slave receives new updates
 * in its output queue. In the WAIT_BGSAVE state instead the server is waiting
 * to start the next background saving in order to send updates to it. */
#define REDIS_REPL_WAIT_BGSAVE_START 6 /* We need to produce a new RDB file. */
#define REDIS_REPL_WAIT_BGSAVE_END 7 /* Waiting RDB file creation to finish. */
#define REDIS_REPL_SEND_BULK 8 /* Sending RDB file to slave. */
#define REDIS_REPL_ONLINE 9 /* RDB file transmitted, sending just updates. */

/* Synchronous read timeout - slave side */
#define REDIS_REPL_SYNCIO_TIMEOUT 5

/* List related stuff */
#define REDIS_HEAD 0
#define REDIS_TAIL 1

/* Sort operations */
#define REDIS_SORT_GET 0
#define REDIS_SORT_ASC 1
#define REDIS_SORT_DESC 2
#define REDIS_SORTKEY_MAX 1024

/* Anti-warning macro... */
#define REDIS_NOTUSED(V) ((void) V)
#define KV_NOTUSED(V) ((void) V)

#define ZSKIPLIST_MAXLEVEL 32 /* Should be enough for 2^32 elements */
#define ZSKIPLIST_P 0.25      /* Skiplist P = 1/4 */

/* Zip structure related defaults */
#define REDIS_HASH_MAX_ZIPLIST_ENTRIES 512
#define REDIS_HASH_MAX_ZIPLIST_VALUE 64
#define REDIS_LIST_MAX_ZIPLIST_ENTRIES 512
#define REDIS_LIST_MAX_ZIPLIST_VALUE 64
#define REDIS_SET_MAX_INTSET_ENTRIES 512
#define REDIS_ZSET_MAX_ZIPLIST_ENTRIES 128
#define REDIS_ZSET_MAX_ZIPLIST_VALUE 64

/* HyperLogLog defines */
#define REDIS_DEFAULT_HLL_SPARSE_MAX_BYTES 3000

/* Sets operations codes */
#define REDIS_OP_UNION 0
#define REDIS_OP_DIFF 1
#define REDIS_OP_INTER 2

/* Redis maxmemory strategies */
#define REDIS_MAXMEMORY_VOLATILE_LRU 0
#define REDIS_MAXMEMORY_VOLATILE_TTL 1
#define REDIS_MAXMEMORY_VOLATILE_RANDOM 2
#define REDIS_MAXMEMORY_ALLKEYS_LRU 3
#define REDIS_MAXMEMORY_ALLKEYS_RANDOM 4
#define REDIS_MAXMEMORY_NO_EVICTION 5
#define REDIS_DEFAULT_MAXMEMORY_POLICY REDIS_MAXMEMORY_NO_EVICTION

/* Units */
#define UNIT_SECONDS 0
#define UNIT_MILLISECONDS 1

/* SHUTDOWN flags */
#define REDIS_SHUTDOWN_SAVE 1       /* Force SAVE on SHUTDOWN even if no save
                                       points are configured. */
#define REDIS_SHUTDOWN_NOSAVE 2     /* Don't SAVE on SHUTDOWN. */

/* Command call flags, see call() function */
#define REDIS_CALL_NONE 0
#define REDIS_CALL_SLOWLOG 1
#define REDIS_CALL_STATS 2
#define REDIS_CALL_PROPAGATE 4
#define REDIS_CALL_FULL (REDIS_CALL_SLOWLOG | REDIS_CALL_STATS | REDIS_CALL_PROPAGATE)

/* Command propagation flags, see propagate() function */
#define REDIS_PROPAGATE_NONE 0
#define REDIS_PROPAGATE_AOF 1
#define REDIS_PROPAGATE_REPL 2

/* RDB active child save type. */
#define REDIS_RDB_CHILD_TYPE_NONE 0
#define REDIS_RDB_CHILD_TYPE_DISK 1     /* RDB is written to disk. */
#define REDIS_RDB_CHILD_TYPE_SOCKET 2   /* RDB is written to slave socket. */

/* Keyspace changes notification classes. Every class is associated with a
 * character for configuration purposes. */
#define REDIS_NOTIFY_KEYSPACE (1<<0)    /* K */
#define REDIS_NOTIFY_KEYEVENT (1<<1)    /* E */
#define REDIS_NOTIFY_GENERIC (1<<2)     /* g */
#define REDIS_NOTIFY_STRING (1<<3)      /* $ */
#define REDIS_NOTIFY_LIST (1<<4)        /* l */
#define REDIS_NOTIFY_SET (1<<5)         /* s */
#define REDIS_NOTIFY_HASH (1<<6)        /* h */
#define REDIS_NOTIFY_ZSET (1<<7)        /* z */
#define REDIS_NOTIFY_EXPIRED (1<<8)     /* x */
#define REDIS_NOTIFY_EVICTED (1<<9)     /* e */
#define REDIS_NOTIFY_ALL (REDIS_NOTIFY_GENERIC | REDIS_NOTIFY_STRING | REDIS_NOTIFY_LIST | REDIS_NOTIFY_SET | REDIS_NOTIFY_HASH | REDIS_NOTIFY_ZSET | REDIS_NOTIFY_EXPIRED | REDIS_NOTIFY_EVICTED)      /* A */

void logicError(const char* fmt, ...);
void logicErrorExpr(int expr, const char* fmt, ...);


/* error code */
#define ERR_NONE           0
#define ERR_TYPE           1
#define ERR_NO_KEY         2
#define ERR_SYNTAX         3
#define ERR_SAME_OBJECT    4
#define ERR_OUT_OF_RANGE   5
#define ERR_NIL            6
#define ERR_NOT_INIT       7
#define ERR_ARGUMENTS      8
#define ERR_PROTOCOL       9
#define ERR_VALUE          10
#define ERR_DB_INDEX       11
#define ERR_CMD            12

/* Macro used to obtain the current LRU clock.
 * If the current resolution is lower than the frequency we refresh the
 * LRU clock (as it should be in production servers) we return the
 * precomputed value, otherwise we need to resort to a function call. */
#define LRU_CLOCK() ((1000/server.hz <= REDIS_LRU_CLOCK_RESOLUTION) ? server.lruclock : getLRUClock())

/* Macro used to initialize a Redis object allocated on the stack.
 * Note that this macro is taken near the structure definition to make sure
 * we'll update it when the structure is changed, to avoid bugs like
 * bug #85 introduced exactly in this way. */
#define initStaticStringObject(_var,_ptr) do {          \
                _var.refcount = 1;                      \
                _var.type = REDIS_STRING;               \
                _var.encoding = REDIS_ENCODING_RAW;     \
                _var.ptr = _ptr;                        \
        } while(0);

/* To improve the quality of the LRU approximation we take a set of keys
 * that are good candidate for eviction across freeMemoryIfNeeded() calls.
 *
 * Entries inside the eviciton pool are taken ordered by idle time, putting
 * greater idle times to the right (ascending order).
 *
 * Empty entries have the key pointer set to NULL. */
#define REDIS_EVICTION_POOL_SIZE 16
struct evictionPoolEntry {
        unsigned long long idle;    /* Object idle time. */
        sds key;                    /* Key name. */
};

/* Redis database representation. There are multiple databases identified
 * by integers from 0 (the default database) up to the max configured
 * database. The database number is the 'id' field in the structure. */
typedef struct redisDb {
        dict *dict;                 /* The keyspace for this DB */
        dict *expires;              /* Timeout of keys with a timeout set */
        dict *blocking_keys;        /* Keys with clients waiting for data (BLPOP) */
        dict *ready_keys;           /* Blocked keys that received a PUSH */
        dict *watched_keys;         /* WATCHED keys for MULTI/EXEC CAS */
        struct evictionPoolEntry *eviction_pool;    /* Eviction pool of keys */
        int id;                     /* Database ID */
        long long avg_ttl;          /* Average TTL, just for stats */
} redisDb;

/* Client MULTI/EXEC state */
typedef struct multiCmd {
        robj **argv;
        int argc;
        struct redisCommand *cmd;
} multiCmd;

typedef struct multiState {
        multiCmd *commands;     /* Array of MULTI commands */
        int count;              /* Total number of MULTI commands */
        int minreplicas;        /* MINREPLICAS for synchronous replication */
        time_t minreplicas_timeout; /* MINREPLICAS timeout as unixtime. */
} multiState;

/* This structure holds the blocking operation state for a client.
 * The fields used depend on client->btype. */
typedef struct blockingState {
        /* Generic fields. */
        mstime_t timeout;       /* Blocking operation timeout. If UNIX current time
                                 * is > timeout then the operation timed out. */

        /* REDIS_BLOCK_LIST */
        dict *keys;             /* The keys we are waiting to terminate a blocking
                                 * operation such as BLPOP. Otherwise NULL. */
        robj *target;           /* The key that should receive the element,
                                 * for BRPOPLPUSH. */

        /* REDIS_BLOCK_WAIT */
        int numreplicas;        /* Number of replicas we are waiting for ACK. */
        long long reploffset;   /* Replication offset to reach. */
} blockingState;

/* The following structure represents a node in the server.ready_keys list,
 * where we accumulate all the keys that had clients blocked with a blocking
 * operation such as B[LR]POP, but received new data in the context of the
 * last executed command.
 *
 * After the execution of every command or script, we run this list to check
 * if as a result we should serve data to clients blocked, unblocking them.
 * Note that server.ready_keys will not have duplicates as there dictionary
 * also called ready_keys in every structure representing a Redis database,
 * where we make sure to remember if a given key was already added in the
 * server.ready_keys list. */
typedef struct readyList {
        redisDb *db;
        robj *key;
} readyList;

/* With multiplexing we need to take per-client state.
 * Clients are taken in a linked list. */
typedef struct redisClient {
        uint64_t id;            /* Client incremental unique ID. */
        int fd;
        redisDb *db;
        int dictid;
        robj *name;             /* As set by CLIENT SETNAME */
        sds querybuf;
        size_t querybuf_peak;   /* Recent (100ms or more) peak of querybuf size */
        int argc;
        robj **argv;
        struct redisCommand *cmd, *lastcmd;
        int reqtype;
        int multibulklen;       /* number of multi bulk arguments left to read */
        long bulklen;           /* length of bulk argument in multi bulk request */
        list *reply;
        unsigned long reply_bytes; /* Tot bytes of objects in reply list */
        int sentlen;            /* Amount of bytes already sent in the current
                                   buffer or object being sent. */
        time_t ctime;           /* Client creation time */
        time_t lastinteraction; /* time of the last interaction, used for timeout */
        time_t obuf_soft_limit_reached_time;
        int flags;              /* REDIS_SLAVE | REDIS_MONITOR | REDIS_MULTI ... */
        int authenticated;      /* when requirepass is non-NULL */
        int replstate;          /* replication state if this is a slave */
        int repl_put_online_on_ack; /* Install slave write handler on ACK. */
        int repldbfd;           /* replication DB file descriptor */
        off_t repldboff;        /* replication DB file offset */
        off_t repldbsize;       /* replication DB file size */
        sds replpreamble;       /* replication DB preamble. */
        long long reploff;      /* replication offset if this is our master */
        long long repl_ack_off; /* replication ack offset, if this is a slave */
        long long repl_ack_time;/* replication ack time, if this is a slave */
        char replrunid[REDIS_RUN_ID_SIZE+1]; /* master run id if this is a master */
        int slave_listening_port; /* As configured with: SLAVECONF listening-port */
        multiState mstate;      /* MULTI/EXEC state */
        int btype;              /* Type of blocking op if REDIS_BLOCKED. */
        blockingState bpop;     /* blocking state */
        long long woff;         /* Last write global replication offset. */
        list *watched_keys;     /* Keys WATCHED for MULTI/EXEC CAS */
        dict *pubsub_channels;  /* channels a client is interested in (SUBSCRIBE) */
        list *pubsub_patterns;  /* patterns a client is interested in (SUBSCRIBE) */
        sds peerid;             /* Cached peer ID. */

        /* Response buffer */
        int bufpos;
        char buf[REDIS_REPLY_CHUNK_BYTES];
} redisClient;

struct saveparam {
        time_t seconds;
        int changes;
};

struct sharedObjectsStruct {
        robj *crlf, *err, *emptybulk, *czero, *cone, *cnegone, *pong, *space,
                *colon, *nullbulk, *nullmultibulk, *queued,
                *emptymultibulk, *nokeyerr, *syntaxerr, *sameobjecterr,
                *outofrangeerr, *noscripterr, *loadingerr, *slowscripterr, *bgsaveerr,
                *masterdownerr, *roslaveerr, *execaborterr, *noautherr, *noreplicaserr,
                *busykeyerr, *oomerr, *plus, *messagebulk, *pmessagebulk, *subscribebulk,
                *unsubscribebulk, *psubscribebulk, *punsubscribebulk, *del, *rpop, *lpop,
                *lpush, *emptyscan, *minstring, *maxstring,
                *select[REDIS_SHARED_SELECT_CMDS],
                *mbulkhdr[REDIS_SHARED_BULKHDR_LEN], /* "*<value>\r\n" */
                *bulkhdr[REDIS_SHARED_BULKHDR_LEN];  /* "$<value>\r\n" */

    
        robj *ok, *wrongtypeerr, *integers[REDIS_SHARED_INTEGERS];
};


/* ZSETs use a specialized version of Skiplists */
typedef struct zskiplistNode {
        robj *obj;
        double score;
        struct zskiplistNode *backward;
        struct zskiplistLevel {
                struct zskiplistNode *forward;
                unsigned int span;
        } level[];
} zskiplistNode;

typedef struct zskiplist {
        struct zskiplistNode *header, *tail;
        unsigned long length;
        int level;
} zskiplist;

typedef struct zset {
        dict *dict;
        zskiplist *zsl;
} zset;

typedef struct clientBufferLimitsConfig {
        unsigned long long hard_limit_bytes;
        unsigned long long soft_limit_bytes;
        time_t soft_limit_seconds;
} clientBufferLimitsConfig;

extern clientBufferLimitsConfig clientBufferLimitsDefaults[REDIS_CLIENT_TYPE_COUNT];

/* The redisOp structure defines a Redis Operation, that is an instance of
 * a command with an argument vector, database ID, propagation target
 * (REDIS_PROPAGATE_*), and command pointer.
 *
 * Currently only used to additionally propagate more commands to AOF/Replication
 * after the propagation of the executed command. */
typedef struct redisOp {
        robj **argv;
        int argc, dbid, target;
        struct redisCommand *cmd;
} redisOp;

/* Defines an array of Redis operations. There is an API to add to this
 * structure in a easy way.
 *
 * redisOpArrayInit();
 * redisOpArrayAppend();
 * redisOpArrayFree();
 */
typedef struct redisOpArray {
        redisOp *ops;
        int numops;
} redisOpArray;

/*-----------------------------------------------------------------------------
 * Global server state
 *----------------------------------------------------------------------------*/

struct clusterState;

/* AIX defines hz to __hz, we don't use this define and in order to allow
 * Redis build on AIX we need to undef it. */
#ifdef _AIX
#undef hz
#endif

struct redisServer {
        /* General */
        pid_t pid;                  /* Main process pid. */
        char *configfile;           /* Absolute config file path, or NULL */
        int hz;                     /* serverCron() calls frequency in hertz */
        redisDb *db;
        unsigned int dbindex;       /* current db index */
        dict *commands;             /* Command table */
        dict *orig_commands;        /* Command table before command renaming. */
        unsigned lruclock:REDIS_LRU_BITS; /* Clock for LRU eviction */
        int shutdown_asap;          /* SHUTDOWN needed ASAP */
        int activerehashing;        /* Incremental rehash in serverCron() */
        char *requirepass;          /* Pass for AUTH command, or NULL */
        char *pidfile;              /* PID file path */
        int arch_bits;              /* 32 or 64 depending on sizeof(long) */
        int cronloops;              /* Number of times the cron function run */
        char runid[REDIS_RUN_ID_SIZE+1];  /* ID always different at every exec. */
        kv_timer_t* timer;          /** timer to delete expire keys */
        /* Networking */
        int port;                   /* TCP listening port */
        int tcp_backlog;            /* TCP listen() backlog */
        char *bindaddr[REDIS_BINDADDR_MAX]; /* Addresses we should bind to */
        int bindaddr_count;         /* Number of addresses in server.bindaddr[] */
        char *unixsocket;           /* UNIX socket path */
        mode_t unixsocketperm;      /* UNIX socket permission */
        int ipfd[REDIS_BINDADDR_MAX]; /* TCP socket file descriptors */
        int ipfd_count;             /* Used slots in ipfd[] */
        int sofd;                   /* Unix socket file descriptor */
        int cfd[REDIS_BINDADDR_MAX];/* Cluster bus listening socket */
        int cfd_count;              /* Used slots in cfd[] */
        list *clients;              /* List of active clients */
        list *clients_to_close;     /* Clients to close asynchronously */
        list *slaves, *monitors;    /* List of slaves and MONITORs */
        redisClient *current_client; /* Current client, only used on crash report */
        int clients_paused;         /* True if clients are currently paused */
        mstime_t clients_pause_end_time; /* Time when we undo clients_paused */
        char neterr[256];   /* Error buffer for anet.c */
        dict *migrate_cached_sockets;/* MIGRATE cached sockets */
        uint64_t next_client_id;    /* Next client unique ID. Incremental. */
        /* RDB / AOF loading information */
        int loading;                /* We are loading data from disk if true */
        off_t loading_total_bytes;
        off_t loading_loaded_bytes;
        time_t loading_start_time;
        off_t loading_process_events_interval_bytes;
        /* Fast pointers to often looked up command */
        struct redisCommand *delCommand, *multiCommand, *lpushCommand, *lpopCommand,
                *rpopCommand;
        /* Fields used only for stats */
        time_t stat_starttime;          /* Server start time */
        long long stat_numcommands;     /* Number of processed commands */
        long long stat_numconnections;  /* Number of connections received */
        long long stat_expiredkeys;     /* Number of expired keys */
        long long stat_evictedkeys;     /* Number of evicted keys (maxmemory) */
        long long stat_keyspace_hits;   /* Number of successful lookups of keys */
        long long stat_keyspace_misses; /* Number of failed lookups of keys */
        size_t stat_peak_memory;        /* Max used memory record */
        long long stat_fork_time;       /* Time needed to perform latest fork() */
        double stat_fork_rate;          /* Fork rate in GB/sec. */
        long long stat_rejected_conn;   /* Clients rejected because of maxclients */
        long long stat_sync_full;       /* Number of full resyncs with slaves. */
        long long stat_sync_partial_ok; /* Number of accepted PSYNC requests. */
        long long stat_sync_partial_err;/* Number of unaccepted PSYNC requests. */
        list *slowlog;                  /* SLOWLOG list of commands */
        long long slowlog_entry_id;     /* SLOWLOG current entry ID */
        long long slowlog_log_slower_than; /* SLOWLOG time limit (to get logged) */
        unsigned long slowlog_max_len;     /* SLOWLOG max number of items logged */
        size_t resident_set_size;       /* RSS sampled in serverCron(). */
        long long stat_net_input_bytes; /* Bytes read from network. */
        long long stat_net_output_bytes; /* Bytes written to network. */
        /* The following two are used to track instantaneous metrics, like
         * number of operations per second, network traffic. */
        struct {
                long long last_sample_time; /* Timestamp of last sample in ms */
                long long last_sample_count;/* Count in last sample */
                long long samples[REDIS_METRIC_SAMPLES];
                int idx;
        } inst_metric[REDIS_METRIC_COUNT];
        /* Configuration */
        int verbosity;                  /* Loglevel in redis.conf */
        int maxidletime;                /* Client timeout in seconds */
        int tcpkeepalive;               /* Set SO_KEEPALIVE if non-zero. */
        int active_expire_enabled;      /* Can be disabled for testing purposes. */
        size_t client_max_querybuf_len; /* Limit for client query buffer length */
        int dbnum;                      /* Total number of configured DBs */
        clientBufferLimitsConfig client_obuf_limits[REDIS_CLIENT_TYPE_COUNT];
        /* AOF persistence */
        int aof_state;                  /* REDIS_AOF_(ON|OFF|WAIT_REWRITE) */
        int aof_fsync;                  /* Kind of fsync() policy */
        char *aof_filename;             /* Name of the AOF file */
        int aof_no_fsync_on_rewrite;    /* Don't fsync if a rewrite is in prog. */
        int aof_rewrite_perc;           /* Rewrite AOF if % growth is > M and... */
        off_t aof_rewrite_min_size;     /* the AOF file is at least N bytes. */
        off_t aof_rewrite_base_size;    /* AOF size on latest startup or rewrite. */
        off_t aof_current_size;         /* AOF current size. */
        int aof_rewrite_scheduled;      /* Rewrite once BGSAVE terminates. */
        pid_t aof_child_pid;            /* PID if rewriting process */
        list *aof_rewrite_buf_blocks;   /* Hold changes during an AOF rewrite. */
        sds aof_buf;      /* AOF buffer, written before entering the event loop */
        int aof_fd;       /* File descriptor of currently selected AOF file */
        int aof_selected_db; /* Currently selected DB in AOF */
        time_t aof_flush_postponed_start; /* UNIX time of postponed AOF flush */
        time_t aof_last_fsync;            /* UNIX time of last fsync() */
        time_t aof_rewrite_time_last;   /* Time used by last AOF rewrite run. */
        time_t aof_rewrite_time_start;  /* Current AOF rewrite start time. */
        int aof_lastbgrewrite_status;   /* REDIS_OK or REDIS_ERR */
        unsigned long aof_delayed_fsync;  /* delayed AOF fsync() counter */
        int aof_rewrite_incremental_fsync;/* fsync incrementally while rewriting? */
        int aof_last_write_status;      /* REDIS_OK or REDIS_ERR */
        int aof_last_write_errno;       /* Valid if aof_last_write_status is ERR */
        int aof_load_truncated;         /* Don't stop on unexpected AOF EOF. */
        /* AOF pipes used to communicate between parent and child during rewrite. */
        int aof_pipe_write_data_to_child;
        int aof_pipe_read_data_from_parent;
        int aof_pipe_write_ack_to_parent;
        int aof_pipe_read_ack_from_child;
        int aof_pipe_write_ack_to_child;
        int aof_pipe_read_ack_from_parent;
        int aof_stop_sending_diff;     /* If true stop sending accumulated diffs
                                          to child process. */
        sds aof_child_diff;             /* AOF diff accumulator child side. */
        /* RDB persistence */
        long long dirty;                /* Changes to DB from the last save */
        long long dirty_before_bgsave;  /* Used to restore dirty on failed BGSAVE */
        pid_t rdb_child_pid;            /* PID of RDB saving child */
        struct saveparam *saveparams;   /* Save points array for RDB */
        int saveparamslen;              /* Number of saving points */
        char *rdb_filename;             /* Name of RDB file */
        int rdb_compression;            /* Use compression in RDB? */
        int rdb_checksum;               /* Use RDB checksum? */
        time_t lastsave;                /* Unix time of last successful save */
        time_t lastbgsave_try;          /* Unix time of last attempted bgsave */
        time_t rdb_save_time_last;      /* Time used by last RDB save run. */
        time_t rdb_save_time_start;     /* Current RDB save start time. */
        int rdb_child_type;             /* Type of save by active child. */
        int lastbgsave_status;          /* REDIS_OK or REDIS_ERR */
        int stop_writes_on_bgsave_err;  /* Don't allow writes if can't BGSAVE */
        int rdb_pipe_write_result_to_parent; /* RDB pipes used to return the state */
        int rdb_pipe_read_result_from_child; /* of each slave in diskless SYNC. */
        /* Propagation of commands in AOF / replication */
        redisOpArray also_propagate;    /* Additional command to propagate. */
        /* Logging */
        char *logfile;                  /* Path of log file */
        int syslog_enabled;             /* Is syslog enabled? */
        char *syslog_ident;             /* Syslog ident */
        int syslog_facility;            /* Syslog facility */
        /* Replication (master) */
        int slaveseldb;                 /* Last SELECTed DB in replication output */
        long long master_repl_offset;   /* Global replication offset */
        int repl_ping_slave_period;     /* Master pings the slave every N seconds */
        char *repl_backlog;             /* Replication backlog for partial syncs */
        long long repl_backlog_size;    /* Backlog circular buffer size */
        long long repl_backlog_histlen; /* Backlog actual data length */
        long long repl_backlog_idx;     /* Backlog circular buffer current offset */
        long long repl_backlog_off;     /* Replication offset of first byte in the
                                           backlog buffer. */
        time_t repl_backlog_time_limit; /* Time without slaves after the backlog
                                           gets released. */
        time_t repl_no_slaves_since;    /* We have no slaves since that time.
                                           Only valid if server.slaves len is 0. */
        int repl_min_slaves_to_write;   /* Min number of slaves to write. */
        int repl_min_slaves_max_lag;    /* Max lag of <count> slaves to write. */
        int repl_good_slaves_count;     /* Number of slaves with lag <= max_lag. */
        int repl_diskless_sync;         /* Send RDB to slaves sockets directly. */
        int repl_diskless_sync_delay;   /* Delay to start a diskless repl BGSAVE. */
        /* Replication (slave) */
        char *masterauth;               /* AUTH with this password with master */
        char *masterhost;               /* Hostname of master */
        int masterport;                 /* Port of master */
        int repl_timeout;               /* Timeout after N seconds of master idle */
        redisClient *master;     /* Client that is master for this slave */
        redisClient *cached_master; /* Cached master to be reused for PSYNC. */
        int repl_syncio_timeout; /* Timeout for synchronous I/O calls */
        int repl_state;          /* Replication status if the instance is a slave */
        off_t repl_transfer_size; /* Size of RDB to read from master during sync. */
        off_t repl_transfer_read; /* Amount of RDB read from master during sync. */
        off_t repl_transfer_last_fsync_off; /* Offset when we fsync-ed last time. */
        int repl_transfer_s;     /* Slave -> Master SYNC socket */
        int repl_transfer_fd;    /* Slave -> Master SYNC temp file descriptor */
        char *repl_transfer_tmpfile; /* Slave-> master SYNC temp file name */
        time_t repl_transfer_lastio; /* Unix time of the latest read, for timeout */
        int repl_serve_stale_data; /* Serve stale data when link is down? */
        int repl_slave_ro;          /* Slave is read only? */
        time_t repl_down_since; /* Unix time at which link with master went down */
        int repl_disable_tcp_nodelay;   /* Disable TCP_NODELAY after SYNC? */
        int slave_priority;             /* Reported in INFO and used by Sentinel. */
        char repl_master_runid[REDIS_RUN_ID_SIZE+1];  /* Master run id for PSYNC. */
        long long repl_master_initial_offset;         /* Master PSYNC offset. */
        /* Replication script cache. */
        dict *repl_scriptcache_dict;        /* SHA1 all slaves are aware of. */
        list *repl_scriptcache_fifo;        /* First in, first out LRU eviction. */
        unsigned int repl_scriptcache_size; /* Max number of elements. */
        /* Synchronous replication. */
        list *clients_waiting_acks;         /* Clients waiting in WAIT command. */
        int get_ack_from_slaves;            /* If true we send REPLCONF GETACK. */
        /* Limits */
        unsigned int maxclients;            /* Max number of simultaneous clients */
        unsigned long long maxmemory;   /* Max number of memory bytes to use */
        int maxmemory_policy;           /* Policy for key eviction */
        int maxmemory_samples;          /* Pricision of random sampling */
        /* Blocked clients */
        unsigned int bpop_blocked_clients; /* Number of clients blocked by lists */
        list *unblocked_clients; /* list of clients to unblock before next loop */
        list *ready_keys;        /* List of readyList structures for BLPOP & co */
        /* Sort parameters - qsort_r() is only available under BSD so we
         * have to take this state global, in order to pass it to sortCompare() */
        int sort_desc;
        int sort_alpha;
        int sort_bypattern;
        int sort_store;
        /* Zip structure config, see redis.conf for more information  */
        size_t hash_max_ziplist_entries;
        size_t hash_max_ziplist_value;
        size_t list_max_ziplist_entries;
        size_t list_max_ziplist_value;
        size_t set_max_intset_entries;
        size_t zset_max_ziplist_entries;
        size_t zset_max_ziplist_value;
        size_t hll_sparse_max_bytes;
        time_t unixtime;        /* Unix time sampled every cron cycle. */
        long long mstime;       /* Like 'unixtime' but with milliseconds resolution. */
        /* Pubsub */
        dict *pubsub_channels;  /* Map channels to list of subscribed clients */
        list *pubsub_patterns;  /* A list of pubsub_patterns */
        int notify_keyspace_events; /* Events to propagate via Pub/Sub. This is an
                                       xor of REDIS_NOTIFY... flags. */
        /* Cluster */
        mstime_t cluster_node_timeout; /* Cluster node timeout. */
        char *cluster_configfile; /* Cluster auto-generated config file name. */
        struct clusterState *cluster;  /* State of the cluster */
        int cluster_migration_barrier; /* Cluster replicas migration barrier. */
        int cluster_slave_validity_factor; /* Slave max data age for failover. */
        int cluster_require_full_coverage; /* If true, put the cluster down if
                                              there is at least an uncovered slot. */
        /* Latency monitor */
        long long latency_monitor_threshold;
        dict *latency_events;
        /* Assert & bug reporting */
        char *assert_failed;
        char *assert_file;
        int assert_line;
        int bug_report_start; /* True if bug report header was already logged. */
        int watchdog_period;  /* Software watchdog period in ms. 0 = off */
};

typedef struct pubsubPattern {
        redisClient *client;
        robj *pattern;
} pubsubPattern;


typedef struct caller{
        int argc;
        robj** argv;
        struct redisCommand* cmd;
        redisDb* db;
        sds proto_text;
        int errnum;
        const char* err;
        list* result; // robj list
}caller_t;


void timer_expire();
void timer_cron();

caller_t* caller_create();
void caller_free_result(caller_t* c);
void caller_free(caller_t* c);
void caller_set_err(caller_t* c, int errnum);
void caller_add_result(caller_t *c, robj *o);
void caller_add_result_long(caller_t *c, long value);
void caller_add_result_str(caller_t *c, const char* str);



//typedef void redisCommandProc(redisClient *c);
typedef void redisCommandProc(caller_t* c);
typedef int *redisGetKeysProc(struct redisCommand *cmd, robj **argv, int argc, int *numkeys);
struct redisCommand {
        char *name;
        redisCommandProc *proc;
        int arity;
        char *sflags; /* Flags as string representation, one char per flag. */
        int flags;    /* The actual flags, obtained from the 'sflags' field. */
        /* Use a function to determine keys arguments in a command line.
         * Used for Redis Cluster redirect. */
        redisGetKeysProc *getkeys_proc;
        /* What keys should be loaded in background when calling this command? */
        int firstkey; /* The first argument that's a key (0 = no keys) */
        int lastkey;  /* The last argument that's a key */
        int keystep;  /* The step between first and last key */
        long long microseconds, calls;
};

struct redisFunctionSym {
        char *name;
        unsigned long pointer;
};

typedef struct _redisSortObject {
        robj *obj;
        union {
                double score;
                robj *cmpobj;
        } u;
} redisSortObject;

typedef struct _redisSortOperation {
        int type;
        robj *pattern;
} redisSortOperation;

/* Structure to hold list iteration abstraction. */
typedef struct {
        robj *subject;
        unsigned char encoding;
        unsigned char direction; /* Iteration direction */
        unsigned char *zi;
        listNode *ln;
} listTypeIterator;

/* Structure for an entry while iterating over a list. */
typedef struct {
        listTypeIterator *li;
        unsigned char *zi;  /* Entry in ziplist */
        listNode *ln;       /* Entry in linked list */
} listTypeEntry;

/* Structure to hold set iteration abstraction. */
typedef struct {
        robj *subject;
        int encoding;
        int ii; /* intset iterator */
        dictIterator *di;
} setTypeIterator;

/* Structure to hold hash iteration abstraction. Note that iteration over
 * hashes involves both fields and values. Because it is possible that
 * not both are required, store pointers in the iterator to avoid
 * unnecessary memory allocation for fields/values. */
typedef struct {
        robj *subject;
        int encoding;

        unsigned char *fptr, *vptr;

        dictIterator *di;
        dictEntry *de;
} hashTypeIterator;

#define REDIS_HASH_KEY 1
#define REDIS_HASH_VALUE 2

/*-----------------------------------------------------------------------------
 * Extern declarations
 *----------------------------------------------------------------------------*/

extern struct redisServer server;
extern struct sharedObjectsStruct shared;
extern dictType setDictType;
extern dictType zsetDictType;
extern dictType clusterNodesDictType;
extern dictType clusterNodesBlackListDictType;
extern dictType dbDictType;
extern dictType shaScriptObjectDictType;
extern double R_Zero, R_PosInf, R_NegInf, R_Nan;
extern dictType hashDictType;
extern dictType replScriptCacheDictType;

/*-----------------------------------------------------------------------------
 * Functions prototypes
 *----------------------------------------------------------------------------*/

/* libkv */
int processProto(caller_t *c);


/* Utils */
long long ustime();
long long mstime();
void getRandomHexChars(char *p, unsigned int len);
uint64_t crc64(uint64_t crc, const unsigned char *s, uint64_t l);
size_t redisPopcount(void *s, long count);

/* networking.c -- Networking and Client related operations */
void *dupClientReplyValue(void *o);
void getClientsMaxBuffers(unsigned long *longest_output_list,
                          unsigned long *biggest_input_buffer);
void formatPeerId(char *peerid, size_t peerid_len, char *ip, int port);
sds getAllClientsInfoString(void);
void freeClientsInAsyncFreeQueue(void);
int getClientTypeByName(char *name);
char *getClientTypeName(int class);
void flushSlavesOutputBuffers(void);
void disconnectSlaves(void);
int listenToPort(int port, int *fds, int *count);
void pauseClients(mstime_t duration);
int clientsArePaused(void);
int processEventsWhileBlocked(void);


/* List data type */
void listTypeTryConversion(robj *subject, robj *value);
void listTypePush(robj *subject, robj *value, int where);
robj *listTypePop(robj *subject, int where);
unsigned long listTypeLength(robj *subject);
listTypeIterator *listTypeInitIterator(robj *subject, long index, unsigned char direction);
void listTypeReleaseIterator(listTypeIterator *li);
int listTypeNext(listTypeIterator *li, listTypeEntry *entry);
robj *listTypeGet(listTypeEntry *entry);
void listTypeInsert(listTypeEntry *entry, robj *value, int where);
int listTypeEqual(listTypeEntry *entry, robj *o);
void listTypeDelete(listTypeEntry *entry);
void listTypeConvert(robj *subject, int enc);
void handleClientsBlockedOnLists(void);
void signalListAsReady(redisDb *db, robj *key);

/* MULTI/EXEC/WATCH... */
void touchWatchedKey(redisDb *db, robj *key);
void touchWatchedKeysOnFlush(int dbid);

/* Redis object implementation */
void decrRefCount(robj *o);
void decrRefCountVoid(void *o);
void incrRefCount(robj *o);
robj *resetRefCount(robj *obj);
void freeStringObject(robj *o);
void freeListObject(robj *o);
void freeSetObject(robj *o);
void freeZsetObject(robj *o);
void freeHashObject(robj *o);
robj *createObject(int type, void *ptr);
robj *createStringObject(char *ptr, size_t len);
robj *createRawStringObject(char *ptr, size_t len);
robj *createEmbeddedStringObject(char *ptr, size_t len);
robj *dupStringObject(robj *o);
int isObjectRepresentableAsLongLong(robj *o, long long *llongval);
robj *tryObjectEncoding(robj *o);
robj *getDecodedObject(robj *o);
size_t stringObjectLen(robj *o);
robj *createStringObjectFromLongLong(long long value);
robj *createStringObjectFromLongDouble(long double value, int humanfriendly);
robj *createListObject(void);
robj *createZiplistObject(void);
robj *createSetObject(void);
robj *createIntsetObject(void);
robj *createHashObject(void);
robj *createZsetObject(void);
robj *createZsetZiplistObject(void);
int checkType(redisClient *c, robj *o, int type);
int getDoubleFromObj(robj *o, double *target);
int getLongLongFromObject(robj *o, long long *target);
int getLongDoubleFromObject(robj *o, long double *target);
char *strEncoding(int encoding);
int compareStringObjects(robj *a, robj *b);
int collateStringObjects(robj *a, robj *b);
int equalStringObjects(robj *a, robj *b);
unsigned long long estimateObjectIdleTime(robj *o);
#define sdsEncodedObject(objptr) (objptr->encoding == REDIS_ENCODING_RAW || objptr->encoding == REDIS_ENCODING_EMBSTR)


/* Struct to hold a inclusive/exclusive range spec by score comparison. */
typedef struct {
    double min, max;
    int minex, maxex; /* are min or max exclusive? */
} zrangespec;

/* Struct to hold an inclusive/exclusive range spec by lexicographic comparison. */
typedef struct {
    robj *min, *max;  /* May be set to shared.(minstring|maxstring) */
    int minex, maxex; /* are min or max exclusive? */
} zlexrangespec;

zskiplist *zslCreate(void);
void zslFree(zskiplist *zsl);
zskiplistNode *zslInsert(zskiplist *zsl, double score, robj *obj);
unsigned char *zzlInsert(unsigned char *zl, robj *ele, double score);
int zslDelete(zskiplist *zsl, double score, robj *obj);
zskiplistNode *zslFirstInRange(zskiplist *zsl, zrangespec *range);
zskiplistNode *zslLastInRange(zskiplist *zsl, zrangespec *range);
double zzlGetScore(unsigned char *sptr);
void zzlNext(unsigned char *zl, unsigned char **eptr, unsigned char **sptr);
void zzlPrev(unsigned char *zl, unsigned char **eptr, unsigned char **sptr);
unsigned int zsetLength(robj *zobj);
void zsetConvert(robj *zobj, int encoding);
unsigned long zslGetRank(zskiplist *zsl, double score, robj *o);

/* Core functions */
struct redisCommand *lookupCommand(sds name);
void propagate(struct redisCommand *cmd, int dbid, robj **argv, int argc, int flags);
void alsoPropagate(struct redisCommand *cmd, int dbid, robj **argv, int argc, int target);
void updateDictResizePolicy(void);
int htNeedsResize(dict *dict);
void oom(const char *msg);
void resetCommandTableStats(void);
void adjustOpenFilesLimit(void);
void closeListeningSockets(int unlink_unix_socket);
void updateCachedTime(void);
void resetServerStats(void);
unsigned int getLRUClock(void);

/* Set data type */
robj *setTypeCreate(robj *value);
int setTypeAdd(robj *subject, robj *value);
int setTypeRemove(robj *subject, robj *value);
int setTypeIsMember(robj *subject, robj *value);
setTypeIterator *setTypeInitIterator(robj *subject);
void setTypeReleaseIterator(setTypeIterator *si);
int setTypeNext(setTypeIterator *si, robj **objele, int64_t *llele);
robj *setTypeNextObject(setTypeIterator *si);
int setTypeRandomElement(robj *setobj, robj **objele, int64_t *llele);
unsigned long setTypeSize(robj *subject);
void setTypeConvert(robj *subject, int enc);

/* Hash data type */
void hashTypeConvert(robj *o, int enc);
void hashTypeTryConversion(robj *subject, robj **argv, int start, int end);
void hashTypeTryObjectEncoding(robj *subject, robj **o1, robj **o2);
robj *hashTypeGetObject(robj *o, robj *key);
int hashTypeExists(robj *o, robj *key);
int hashTypeSet(robj *o, robj *key, robj *value);
int hashTypeDelete(robj *o, robj *key);
unsigned long hashTypeLength(robj *o);
hashTypeIterator *hashTypeInitIterator(robj *subject);
void hashTypeReleaseIterator(hashTypeIterator *hi);
int hashTypeNext(hashTypeIterator *hi);
void hashTypeCurrentFromZiplist(hashTypeIterator *hi, int what,
                                unsigned char **vstr,
                                unsigned int *vlen,
                                long long *vll);
void hashTypeCurrentFromHashTable(hashTypeIterator *hi, int what, robj **dst);
robj *hashTypeCurrentObject(hashTypeIterator *hi, int what);

/* Pub / Sub */
void freePubsubPattern(void *p);
int listMatchPubsubPattern(void *a, void *b);
int pubsubPublishMessage(robj *channel, robj *message);

/* db.c -- Keyspace access API */
robj *lookupKey(redisDb *db, robj *key);
robj *lookupKeyRead(redisDb *db, robj *key);
robj *lookupKeyWrite(redisDb *db, robj *key);
void dbAdd(redisDb *db, robj *key, robj *val);
void dbOverwrite(redisDb *db, robj *key, robj *val);
void setKey(redisDb *db, robj *key, robj *val);
int dbExists(redisDb *db, robj *key);
int dbDelete(redisDb *db, robj *key);
robj *dbUnshareStringValue(redisDb *db, robj *key, robj *o);
long long emptyDb(void(callback)(void*));
int selectDb(caller_t *c, int id);
void signalModifiedKey(redisDb *db, robj *key);
void signalFlushedDb(int dbid);
int verifyClusterConfigWithData(void);

/** expire */
void setExpire(redisDb *db, robj *key, long long when);
int expireIfNeeded(redisDb *db, robj *key);
int removeExpire(redisDb *db, robj *key);

/* API to get key arguments from commands */
int *getKeysFromCommand(struct redisCommand *cmd, robj **argv, int argc, int *numkeys);
void getKeysFreeResult(int *result);
int *zunionInterGetKeys(struct redisCommand *cmd,robj **argv, int argc, int *numkeys);
int *evalGetKeys(struct redisCommand *cmd, robj **argv, int argc, int *numkeys);
int *sortGetKeys(struct redisCommand *cmd, robj **argv, int argc, int *numkeys);

/* Scripting */
void scriptingInit(void);

/* Git SHA1 */
char *redisGitSHA1(void);
char *redisGitDirty(void);
uint64_t redisBuildId(void);

/* Commands prototypes */
void getCommand(caller_t *c);
void setCommand(caller_t *c);
void delCommand(caller_t *c);
void incrCommand(caller_t *c);
void incrbyCommand(caller_t *c);
void decrCommand(caller_t *c);
void decrbyCommand(caller_t *c);
void lpushCommand(caller_t *c);
void lrangeCommand(caller_t *c);
void saddCommand(caller_t *c);
void smembersCommand(caller_t *c);
void zaddCommand(caller_t *c);
void zrangeCommand(caller_t *c);
void hsetCommand(caller_t *c);
void hgetCommand(caller_t *c);
void existsCommand(caller_t *c);
void scardCommand(caller_t *c);
void selectCommand(caller_t *c);
void sismemberCommand(caller_t *c);
void typeCommand(caller_t *c);
void zcardCommand(caller_t *c);
void dbsizeCommand(caller_t *c);
void flushdbCommand(caller_t *c);
void expireCommand(caller_t *c);
void expireatCommand(caller_t *c);
void pexpireCommand(caller_t *c);
void pexpireatCommand(caller_t *c);

#define KV_DEFAULT_DBNUM     16


int kv_init_main();

//void kv_config(const char* param1, ...); // eg: kv_config("maxmemory", "1024", "dbsize", "16");


#endif
