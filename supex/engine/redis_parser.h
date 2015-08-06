#pragma once

#define	MAX_KV_PAIRS		30
#define	MAX_KV_PAIRS_INDEX	(MAX_KV_PAIRS - 1)

#define OPT_OK			"+OK\r\n"
#define OPT_FAILED			"-FAILED\r\n"
#define OPT_BULK_NULL		"$-1\r\n"
#define OPT_MULTI_BULK_NULL	"*0\r\n"
#define OPT_MULTI_BULK_FALSE	"*-1\r\n"
#define OPT_CMD_ERROR		"-CMD ERROR\r\n"
#define OPT_NO_THIS_CMD		"-NO THIS COMMAND\r\n"
#define OPT_KV_TOO_MUCH		"-KEY VALUE TOO MUCH\r\n"
#define OPT_NAME_TOO_LONG	"-NAME TOO LONG\r\n"
#define OPT_UNKNOW_ERROR	"-UNKNOW ERROR CODE\r\n"
#define OPT_INTERIOR_ERROR	"-INTERIOR ERROR!\r\n"
#define OPT_NO_MEMORY		"-NO MORE MEMORY!\r\n"
#define OPT_DATA_TOO_LARGE	"-DATA TOO LARGE!\r\n"



/**
 * Protocol to reply
 */
/** type definition */
#define REDIS_REPLY_STRING 1
#define REDIS_REPLY_ARRAY 2
#define REDIS_REPLY_INTEGER 3
#define REDIS_REPLY_NIL 4
#define REDIS_REPLY_STATUS 5
#define REDIS_REPLY_ERROR 6

/** error definition */
#define REDIS_ERROR_OOM      1
#define REDIS_ERROR_PROTOCOL 2
#define REDIS_ERROR_OTHER    3 



struct redis_parser {
	int kvs;
	int cmd;
	size_t klen;
	size_t vlen;
	size_t flen;
	
	int doptr;
	int key;
	int val;
	int fld;
};

struct redis_status {
	size_t klen_array[ MAX_KV_PAIRS ];
	size_t vlen_array[ MAX_KV_PAIRS ];
	size_t flen_array[ MAX_KV_PAIRS ];

	int key_offset[ MAX_KV_PAIRS ];
	int val_offset[ MAX_KV_PAIRS ];
	int fld_offset[ MAX_KV_PAIRS ];

	int keys;
	int vals;
	int flds;
	int order;
};

struct redis_parse_info {
	struct redis_parser rp;
	struct redis_status rs;
};

struct redis_reply {
        int type;  /* REDIS_REPLY_* */
        int err;
        int len;  /* Length of string */
        char *str;/* Used for both REDIS_REPLY_ERROR and REDIS_REPLY_STRING */
        long long integer;
        
        size_t elements; /* number of elements, for REDIS_REPLY_ARRAY */
        struct redis_reply **element; /* elements vector for REDIS_REPLY_ARRAY */
};


/**
 * @return Redis protocol string, it's your responsibility to free it when needed.
 *         NULL if invalid command(paramter) passed.
 */
int cmd_to_proto(char **cmd, const char *fmt, ...);

int first_reply_ok(const char *buf, size_t buflen, size_t *reply_len);
        
/**
 * Convert redis protocol to redis_reply structure.
 */
struct redis_reply* proto_to_reply(const char *proto, size_t proto_len);
void redis_reply_release(struct redis_reply *reply);
