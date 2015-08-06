/**
 * Copyright (c) 2015
 * All Rights Reserved
 *
 * @file    libkv.h
 * @detail  libkv API header file.
 *
 * @author  shishengjie
 * @date    2015-05-12
 *
 *
 * @mainpage Libkv references
 *           <pre>
 * <b>libkv command list</b>
 *
 *           - <b>set key value</b>
 *             Description
 *                    set string value of a key.
 *             Return value
 *                    success:"OK".
 *                    failed :error code.
 *                    
 *           - <b>del key [key ...]</b>
 *             Description
 *                    Delete a key
 *             Return value
 *                    success:number of deleted.
 *                    failed :error code.
 *                    
 *           - <b>get key</b>
 *             Description
 *                    Get string value of a key.
 *             Return value
 *                    success:the value of key.
 *                    failed :error code.
 *                    
 *           - <b>dbsize</b>
 *             Description
 *                    Get the number of keys in the select db.
 *             Return value
 *                    success:the number of keys in the select db.
 *                    failed :error code.
 *                    
 *           - <b>flushdb</b>
 *             Description
 *                    Remove all keys from the current database.
 *             Return value
 *                    success:"OK"
 *                    failed :error code.
 *                    
 *           - <b>incr key</b>
 *             Description
 *                    Increment the integer value of a key by one.
 *             Return value
 *                    success:the value of key after the increment.
 *                    failed :error code.
 *
 *           - <b>incrby key increment</b>
 *             Description
 *                    Increment the integer value of a key by the given amount.
 *             Return value
 *                    success:the value of key after the increment.
 *                    failed :error code.
 *                    
 *           - <b>decr key</b>
 *             Description
 *                    Decrement integer value of key by one.
 *             Return value
 *                    success:the value of key after decrement. 
 *                    failed :error code.
 *
 *           - <b>decrby key decrement</b>
 *             Description
 *                    Decrement the integer value of a key by the given number.
 *             Return value
 *                    success:the value of key after the decrement.
 *                    failed :error code.
 *
 *           - <b>lpush key value [value ...]</b>
 *             Description
 *                    Prepend one or multiple values to a list.
 *             Return value
 *                    success:the length of the list after the push operations.
 *                    failed :error code.
 *
 *           - <b>exists key</b>
 *             Description
 *                    Determine if a key exists
 *             Return value
 *                    success:return 1 if exist, otherwise 0.
 *                    failed :error code.
 *
 *           - <b>lrange key start stop</b>
 *             Description
 *                    Get a range of elements from a list.
 *             Return value
 *                    success:An elements array which in the specified range.
 *                    failed :error code.
 *
 *           - <b>sadd key [key ...]</b>
 *             Description
 *                    Add one or more members to a set.
 *             Return value
 *                    success:The number of elements that were added to the set.
 *                    failed :error code.
 *
 *           - <b>smembers key</b>
 *             Description
 *                    Get all elements from a set.
 *             Return value
 *                    success:return all elements of the specified set(key). 
 *                    failed :error code.
 *
 *           - <b>expire key seconds</b>
 *             Description
 *                    Set a key's time to live in seconds
 *             Return value
 *                    success:Return 1 if timeout was set,
 *                            return 0 if key not exist or could not be set.
 *                    failed :error code.
 *
 *           - <b>expireat key timestamp</b>
 *             Description
 *                    Set a key's time to live in timestamp seconds.
 *             Return value
 *                    success:Return 1 if timeout was set,
 *                            return 0 if key not exist or could not be set.
 *                    failed :error code.
 *
 *           - <b>pexpire key milliseconds</b>
 *             Description
 *                    Set a key's time to live in milliseconds.
 *             Return value
 *                    success:Return 1 if timeout was set,
 *                            return 0 if key not exist or could not be set.
 *                    failed :error code.
 *
 *           - <b>pexpireat key milliseconds-timestamp</b>
 *             Description
 *                    Set a key's time to live in milliseconds-timestamp.
 *             Return value
 *                    success:Return 1 if timeout was set,
 *                            return 0 if key not exist or could not be set.
 *                    failed :error code.
 *
 *           </pre>
 */

#ifndef LIBKV_H_
#define LIBKV_H_



#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup group_error_code Error code
 * @{
 * User level error code.
 */
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
/**
 * @}
 */
        
/**
 * @defgroup group_result_container Result container
 * Contain command execution results.
 * @{
 */

/** result list iterator from head */
#define ANSWER_HEAD 0
/** result list iterator from tail */
#define ANSWER_TAIL 1



        /**
         * answer_t container node entry.
         */
        typedef struct answer_value {
                void *ptr;
                unsigned long ptrlen;
                struct answer_value* prev;
                struct answer_value* next;
        }answer_value_t;

        /**
         * answer_t container list iterator. 
         */
        typedef struct answer_iterator {
                answer_value_t *next;
                int direction;
        }answer_iter_t;

        /**
         * answer_t container(list).
         */
        typedef struct answer {
                int errnum;
                const char* err;
                unsigned long count; 
                struct answer_value* head, *tail;
        }answer_t;
    
    
        /**
         * Create new answer_t structure
         * @return A new malloc initailized answer_t returned, otherwise NULL. 
         */
        answer_t* answer_create();

        /**
         * Free answer_t structure, per node->ptr and per node.
         * @param a answer_t pointer to free.
         */
        void answer_release(answer_t *a);

        /**
         * Get the value number of list in answer.
         * @param a Which answer to get from.
         * @return Value number in list contain.
         */
        unsigned long answer_length(answer_t *a);
        
        /**
         * Malloc a new value entry and initialized by ptr, ptrlen.
         * @param ptr Value entry holds content.
         * @param ptrlen ptr length.
         * @return A new malloc value entry.
         */
        answer_value_t* answer_value_create(const void *ptr, unsigned long ptrlen);

        /**
         * Add a new value entry to the tail of answer_t list
         * @param a answer_t container
         * @param ptr The added content pointer.
         * @param ptrlen The length of pointer to the content.
         */
        void answer_add_value_tail(answer_t *a, const void *ptr, unsigned long ptrlen);

        /**
         * Add a new value entry to the head of answer_t list
         * @param a answer_t container
         * @param ptr The added content pointer.
         * @param ptrlen The length of pointer to the content.
         */
        void answer_add_value_head(answer_t *a, const void *ptr, unsigned long ptrlen);

        /**
         * Get the iterator of answer_t. Overall nodes through it.
         * @param a answer_t where iterator from.
         * @param direction Ensure direction start from head or tail.
         * @return A new malloc iterator of a's. 
         */
        answer_iter_t* answer_get_iter(answer_t *a, int direction);

        /**
         * Rewind iterator to start position.
         * @param a answer_t where iterator from.
         * @param iter Reset iterator to start position.
         */
        void answer_rewind_iter(answer_t *a, answer_iter_t *iter);

        /**
         * Free iterator.
         * @param iter Iterator to be freed.
         */
        void answer_release_iter(answer_iter_t *iter);

        /**
         * Get current value entry and move iterator to the next.
         * @param iter Iterator where value from.
         * @return return current iterator's value pointer.
         */
        answer_value_t* answer_next(answer_iter_t *iter);

        /**
         * Get first value entry pointer.
         * @param ans answer_t where get from.
         * @return First value entry pointer or NULL if empty list.
         */
        answer_value_t* answer_first_value(answer_t *ans);

        /**
         * Get last value entry pointer.
         * @param ans answer_t where get from.
         * @return Last value entry pointer or NULL if empty list.
         */
        answer_value_t* answer_last_value(answer_t *ans);

        /**
         * Convert value->ptr to a null-terminated string,
         * append '\0' character to value->ptr.
         * @param value Value entry pointer to be converted.
         * @return Force to convert to char* type and return.
         */
        char* answer_value_to_string(answer_value_t *value);
/**
 *@}
 */

        
        /**
         * Initialize libkv that you must do it at first! You can pass param-value pairs
         * or a single parameter like:
         *                        kv_init("enable_rdb",
         *                                "rdb_file", "./dump.rdb",
         *                                ...
         *                               );
         * @param  param1,... Parameters list(not support currently, just pass NULL instead).
         * @return success Return ERR_NONE, otherwise error code.
         */
        int kv_init(const char *param1, ...);

        /**
         * Free libkv resources if you want to do nothing next with libkv.
         */
        void kv_uninit();

        /**
         * Command executes interface.
         * @param  cmd Specified length command bytes array.
         * @param  cmdlen The length(bytes) of command bytes array.
         * @return answer_t The structure contains command execution result.
         */
        answer_t* kv_ask(const char *cmd, unsigned int cmdlen);

        /**
         * Get libkv version.
         * @return libkv version string
         */
        const char* kv_version();

        /**
         * Get libkv version numerice. Format [112233]->[11-major 22-minor 33-revision]
         * @return A long type number indicates version.
         */
        long kv_version_numeric();

        /**
         * Get current memory used by libkv
         * @return Bytes used by libkv.
         */
        unsigned int kv_get_used_memory();

        

#ifdef __cplusplus
}
#endif


#endif
