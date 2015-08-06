#ifndef TEST_REDIS_REPLY_H_
#define TEST_REDIS_REPLY_H_



#include <stdio.h>



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



struct redis_reply {
        int type;  /* REDIS_REPLY_* */
        int err;
        int len;  /* Length of string */
        char *str;/* Used for both REDIS_REPLY_ERROR and REDIS_REPLY_STRING */
        long long integer;
        
        size_t elements; /* number of elements, for REDIS_REPLY_ARRAY */
        struct redis_reply **element; /* elements vector for REDIS_REPLY_ARRAY */
};



int first_reply_ok(const char *buf, size_t buflen, size_t *reply_len);
        
/**
 * Convert redis protocol to redis_reply structure.
 */
struct redis_reply* proto_to_reply(const char *proto, size_t proto_len);
void redis_reply_release(struct redis_reply *reply);



#endif
