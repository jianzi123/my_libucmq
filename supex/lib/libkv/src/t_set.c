/*
 * Copyright (c) 2009-2012, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "kv_inner.h"

/*-----------------------------------------------------------------------------
 * Set Commands
 *----------------------------------------------------------------------------*/

void sunionDiffGenericCommand(redisClient *c, robj **setkeys, int setnum, robj *dstkey, int op);

/* Factory method to return a set that *can* hold "value". When the object has
 * an integer-encodable value, an intset will be returned. Otherwise a regular
 * hash table. */
robj *setTypeCreate(robj *value) {
    if (isObjectRepresentableAsLongLong(value,NULL) == REDIS_OK)
        return createIntsetObject();
    return createSetObject();
}

int setTypeAdd(robj *subject, robj *value) {
    long long llval;
    if (subject->encoding == REDIS_ENCODING_HT) {
        if (dictAdd(subject->ptr,value,NULL) == DICT_OK) {
            incrRefCount(value);
            return 1;
        }
    } else if (subject->encoding == REDIS_ENCODING_INTSET) {
        if (isObjectRepresentableAsLongLong(value,&llval) == REDIS_OK) {
            uint8_t success = 0;
            subject->ptr = intsetAdd(subject->ptr,llval,&success);
            if (success) {
                /* Convert to regular set when the intset contains
                 * too many entries. */
                if (intsetLen(subject->ptr) > server.set_max_intset_entries)
                    setTypeConvert(subject,REDIS_ENCODING_HT);
                return 1;
            }
        } else {
            /* Failed to get integer from object, convert to regular set. */
            setTypeConvert(subject,REDIS_ENCODING_HT);

            /* The set *was* an intset and this value is not integer
             * encodable, so dictAdd should always work. */
            logicErrorExpr(dictAdd(subject->ptr,value,NULL) == DICT_OK, "Never happend");
            incrRefCount(value);
            return 1;
        }
    } else {
        logicError("Unknown set encoding");
    }
    return 0;
}

int setTypeRemove(robj *setobj, robj *value) {
    long long llval;
    if (setobj->encoding == REDIS_ENCODING_HT) {
        if (dictDelete(setobj->ptr,value) == DICT_OK) {
            if (htNeedsResize(setobj->ptr)) dictResize(setobj->ptr);
            return 1;
        }
    } else if (setobj->encoding == REDIS_ENCODING_INTSET) {
        if (isObjectRepresentableAsLongLong(value,&llval) == REDIS_OK) {
            int success;
            setobj->ptr = intsetRemove(setobj->ptr,llval,&success);
            if (success) return 1;
        }
    } else {
        logicError("Unknown set encoding");
    }
    return 0;
}

int setTypeIsMember(robj *subject, robj *value) {
    long long llval;
    if (subject->encoding == REDIS_ENCODING_HT) {
        return dictFind((dict*)subject->ptr,value) != NULL;
    } else if (subject->encoding == REDIS_ENCODING_INTSET) {
        if (isObjectRepresentableAsLongLong(value,&llval) == REDIS_OK) {
            return intsetFind((intset*)subject->ptr,llval);
        }
    } else {
        logicError("Unknown set encoding");
    }
    return 0;
}

setTypeIterator *setTypeInitIterator(robj *subject) {
    setTypeIterator *si = zmalloc(sizeof(setTypeIterator));
    si->subject = subject;
    si->encoding = subject->encoding;
    if (si->encoding == REDIS_ENCODING_HT) {
        si->di = dictGetIterator(subject->ptr);
    } else if (si->encoding == REDIS_ENCODING_INTSET) {
        si->ii = 0;
    } else {
        logicError("Unknown set encoding");
    }
    return si;
}

void setTypeReleaseIterator(setTypeIterator *si) {
    if (si->encoding == REDIS_ENCODING_HT)
        dictReleaseIterator(si->di);
    zfree(si);
}

/* Move to the next entry in the set. Returns the object at the current
 * position.
 *
 * Since set elements can be internally be stored as redis objects or
 * simple arrays of integers, setTypeNext returns the encoding of the
 * set object you are iterating, and will populate the appropriate pointer
 * (eobj) or (llobj) accordingly.
 *
 * When there are no longer elements -1 is returned.
 * Returned objects ref count is not incremented, so this function is
 * copy on write friendly. */
int setTypeNext(setTypeIterator *si, robj **objele, int64_t *llele) {
    if (si->encoding == REDIS_ENCODING_HT) {
        dictEntry *de = dictNext(si->di);
        if (de == NULL) return -1;
        *objele = dictGetKey(de);
    } else if (si->encoding == REDIS_ENCODING_INTSET) {
        if (!intsetGet(si->subject->ptr,si->ii++,llele))
            return -1;
    }
    return si->encoding;
}

/* The not copy on write friendly version but easy to use version
 * of setTypeNext() is setTypeNextObject(), returning new objects
 * or incrementing the ref count of returned objects. So if you don't
 * retain a pointer to this object you should call decrRefCount() against it.
 *
 * This function is the way to go for write operations where COW is not
 * an issue as the result will be anyway of incrementing the ref count. */
robj *setTypeNextObject(setTypeIterator *si) {
    int64_t intele;
    robj *objele;
    int encoding;

    encoding = setTypeNext(si,&objele,&intele);
    switch(encoding) {
        case -1:    return NULL;
        case REDIS_ENCODING_INTSET:
            return createStringObjectFromLongLong(intele);
        case REDIS_ENCODING_HT:
            incrRefCount(objele);
            return objele;
        default:
            logicError("Unsupported encoding");
    }
    return NULL; /* just to suppress warnings */
}

/* Return random element from a non empty set.
 * The returned element can be a int64_t value if the set is encoded
 * as an "intset" blob of integers, or a redis object if the set
 * is a regular set.
 *
 * The caller provides both pointers to be populated with the right
 * object. The return value of the function is the object->encoding
 * field of the object and is used by the caller to check if the
 * int64_t pointer or the redis object pointer was populated.
 *
 * When an object is returned (the set was a real set) the ref count
 * of the object is not incremented so this function can be considered
 * copy on write friendly. */
int setTypeRandomElement(robj *setobj, robj **objele, int64_t *llele) {
    if (setobj->encoding == REDIS_ENCODING_HT) {
        dictEntry *de = dictGetRandomKey(setobj->ptr);
        *objele = dictGetKey(de);
    } else if (setobj->encoding == REDIS_ENCODING_INTSET) {
        *llele = intsetRandom(setobj->ptr);
    } else {
        logicError("Unknown set encoding");
    }
    return setobj->encoding;
}

unsigned long setTypeSize(robj *subject) {
    if (subject->encoding == REDIS_ENCODING_HT) {
        return dictSize((dict*)subject->ptr);
    } else if (subject->encoding == REDIS_ENCODING_INTSET) {
        return intsetLen((intset*)subject->ptr);
    } else {
        logicError("Unknown set encoding");
    }

    return 0;
}

/* Convert the set to specified encoding. The resulting dict (when converting
 * to a hash table) is presized to hold the number of elements in the original
 * set. */
void setTypeConvert(robj *setobj, int enc) {
    setTypeIterator *si;
    logicErrorExpr(setobj->type == REDIS_SET &&
                   setobj->encoding == REDIS_ENCODING_INTSET, "Never happend");

    if (enc == REDIS_ENCODING_HT) {
        int64_t intele;
        dict *d = dictCreate(&setDictType,NULL);
        robj *element;

        /* Presize the dict to avoid rehashing */
        dictExpand(d,intsetLen(setobj->ptr));

        /* To add the elements we extract integers and create redis objects */
        si = setTypeInitIterator(setobj);
        while (setTypeNext(si,NULL,&intele) != -1) {
            element = createStringObjectFromLongLong(intele);
            logicErrorExpr(dictAdd(d,element,NULL) == DICT_OK, "Never happend");
        }
        setTypeReleaseIterator(si);

        setobj->encoding = REDIS_ENCODING_HT;
        zfree(setobj->ptr);
        setobj->ptr = d;
    } else {
            logicError("Unsupported set conversion");
    }
}

void saddCommand(caller_t* c)
{
    int i;
    unsigned long added = 0;
    robj *set,*result;

    set = lookupKeyWrite(c->db, c->argv[1]);
    if (set == NULL) {
        set = setTypeCreate(c->argv[2]);
        dbAdd(c->db, c->argv[1], set);
    } else {
        if (set->type != REDIS_SET) {
            caller_set_err(c, ERR_TYPE);
            return ;
        }
    }

    for (i = 2; i < c->argc; i++) {
        c->argv[i] = tryObjectEncoding(c->argv[i]);
        if (setTypeAdd(set,c->argv[i]))
                added++;
    }

    server.dirty += added;

    if (added < REDIS_SHARED_INTEGERS) {
            result = shared.integers[added];
            incrRefCount(shared.integers[added]);
    } else {
            result = createObject(REDIS_STRING, (void*)added);
            result->encoding = REDIS_ENCODING_INT;
    }

    caller_add_result(c, result);
    return ;
}



void sismemberCommand(caller_t *c)
{
        robj *set;

        if ((set = lookupKeyRead(c->db, c->argv[1])) == NULL || set->type != REDIS_SET) {
                caller_add_result_long(c, 0);
                return;
        }

        c->argv[2] = tryObjectEncoding(c->argv[2]);
        if (setTypeIsMember(set, c->argv[2])) {
                caller_add_result_long(c, 1);
                return; 
        } else {
                caller_add_result_long(c, 0);
                return;
        }
}

void scardCommand(caller_t *c)
{
        robj *o, *result;

        o = lookupKeyRead(c->db, c->argv[1]);
        if (o == NULL)
                result = createObject(REDIS_STRING, (void*)0);
        else
                result = createObject(REDIS_STRING, (void*)setTypeSize(o));

        result->encoding = REDIS_ENCODING_INT;
        caller_add_result(c, result);
}



/* handle the "SRANDMEMBER key <count>" variant. The normal version of the
 * command is handled by the srandmemberCommand() function itself. */

/* How many times bigger should be the set compared to the requested size
 * for us to don't use the "remove elements" strategy? Read later in the
 * implementation for more info. */
#define SRANDMEMBER_SUB_STRATEGY_MUL 3


int qsortCompareSetsByCardinality(const void *s1, const void *s2) {
    return setTypeSize(*(robj**)s1)-setTypeSize(*(robj**)s2);
}

/* This is used by SDIFF and in this case we can receive NULL that should
 * be handled as empty sets. */
int qsortCompareSetsByRevCardinality(const void *s1, const void *s2) {
    robj *o1 = *(robj**)s1, *o2 = *(robj**)s2;

    return  (o2 ? setTypeSize(o2) : 0) - (o1 ? setTypeSize(o1) : 0);
}

void _smembersCommand(caller_t *c, robj **setkeys, unsigned long setnum)
{
        KV_NOTUSED(setnum);
        setTypeIterator *si;
        robj *result;
        robj *eleobj;
        int64_t intobj;
        int encoding;
        robj *setobj = lookupKeyRead(c->db, setkeys[0]);
        if (!setobj) {
                caller_set_err(c, ERR_NIL);
                return;
        }

        if (setobj->type != REDIS_SET) {
                caller_set_err(c, ERR_TYPE);
                return;
        }

        si = setTypeInitIterator(setobj);
        while((encoding = setTypeNext(si,&eleobj,&intobj)) != -1) {
                if (encoding == REDIS_ENCODING_HT) {
                        caller_add_result(c, eleobj);
                        incrRefCount(eleobj);
                } else { // intset
                        if (intobj >= 0 && intobj < REDIS_SHARED_INTEGERS) {
                                result = shared.integers[intobj];
                                incrRefCount(result);
                        } else {
                                result = createObject(REDIS_STRING, (void*)intobj);
                                result->encoding = REDIS_ENCODING_INT;
                        }
                        caller_add_result(c, result);
                }
        }
        setTypeReleaseIterator(si);
}

void smembersCommand(caller_t *c)
{
    _smembersCommand(c, c->argv+1, c->argc-1);
}

#define REDIS_OP_UNION 0
#define REDIS_OP_DIFF 1
#define REDIS_OP_INTER 2
