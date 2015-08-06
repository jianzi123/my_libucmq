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
 * List API
 *----------------------------------------------------------------------------*/

/* Check the argument length to see if it requires us to convert the ziplist
 * to a real list. Only check raw-encoded objects because integer encoded
 * objects are never too long. */
void listTypeTryConversion(robj *subject, robj *value) {
    if (subject->encoding != REDIS_ENCODING_ZIPLIST) return;
    if (sdsEncodedObject(value) &&
        sdslen(value->ptr) > server.list_max_ziplist_value)
            listTypeConvert(subject,REDIS_ENCODING_LINKEDLIST);
}

/* The function pushes an element to the specified list object 'subject',
 * at head or tail position as specified by 'where'.
 *
 * There is no need for the caller to increment the refcount of 'value' as
 * the function takes care of it if needed. */
void listTypePush(robj *subject, robj *value, int where) {
        /** Check if we need to convert the ziplist */
        listTypeTryConversion(subject,value);
        if (subject->encoding == REDIS_ENCODING_ZIPLIST &&
            ziplistLen(subject->ptr) >= server.list_max_ziplist_entries)
                listTypeConvert(subject,REDIS_ENCODING_LINKEDLIST);

        if (subject->encoding == REDIS_ENCODING_ZIPLIST) {
                int pos = (where == REDIS_HEAD) ? ZIPLIST_HEAD : ZIPLIST_TAIL;
                value = getDecodedObject(value);
                subject->ptr = ziplistPush(subject->ptr,value->ptr,sdslen(value->ptr),pos);
                decrRefCount(value);
        } else if (subject->encoding == REDIS_ENCODING_LINKEDLIST) {
                if (where == REDIS_HEAD) {
                        listAddNodeHead(subject->ptr,value);
                } else {
                        listAddNodeTail(subject->ptr,value);
                }
                incrRefCount(value);
        } else {
                logicError("Unknown list encoding");
        }
}

robj *listTypePop(robj *subject, int where) {
    robj *value = NULL;
    if (subject->encoding == REDIS_ENCODING_ZIPLIST) {
        unsigned char *p;
        unsigned char *vstr;
        unsigned int vlen;
        long long vlong;
        int pos = (where == REDIS_HEAD) ? 0 : -1;
        p = ziplistIndex(subject->ptr,pos);
        if (ziplistGet(p,&vstr,&vlen,&vlong)) {
            if (vstr) {
                value = createStringObject((char*)vstr,vlen);
            } else {
                value = createStringObjectFromLongLong(vlong);
            }
            /* We only need to delete an element when it exists */
            subject->ptr = ziplistDelete(subject->ptr,&p);
        }
    } else if (subject->encoding == REDIS_ENCODING_LINKEDLIST) {
        list *list = subject->ptr;
        listNode *ln;
        if (where == REDIS_HEAD) {
            ln = listFirst(list);
        } else {
            ln = listLast(list);
        }
        if (ln != NULL) {
            value = listNodeValue(ln);
            incrRefCount(value);
            listDelNode(list,ln);
        }
    } else {
        logicError("Unknown list encoding");
    }
    return value;
}

unsigned long listTypeLength(robj *subject) {
    if (subject->encoding == REDIS_ENCODING_ZIPLIST) {
        return ziplistLen(subject->ptr);
    } else if (subject->encoding == REDIS_ENCODING_LINKEDLIST) {
        return listLength((list*)subject->ptr);
    } else {
        logicError("Unknown list encoding");
    }

    return 0;
}

/* Initialize an iterator at the specified index. */
listTypeIterator *listTypeInitIterator(robj *subject, long index, unsigned char direction) {
    listTypeIterator *li = zmalloc(sizeof(listTypeIterator));
    li->subject = subject;
    li->encoding = subject->encoding;
    li->direction = direction;
    if (li->encoding == REDIS_ENCODING_ZIPLIST) {
        li->zi = ziplistIndex(subject->ptr,index);
    } else if (li->encoding == REDIS_ENCODING_LINKEDLIST) {
        li->ln = listIndex(subject->ptr,index);
    } else {
        logicError("Unknown list encoding");
    }
    return li;
}

/* Clean up the iterator. */
void listTypeReleaseIterator(listTypeIterator *li) {
    zfree(li);
}

/* Stores pointer to current the entry in the provided entry structure
 * and advances the position of the iterator. Returns 1 when the current
 * entry is in fact an entry, 0 otherwise. */
int listTypeNext(listTypeIterator *li, listTypeEntry *entry) {
    /* Protect from converting when iterating */
        logicErrorExpr(li->subject->encoding == li->encoding, "Invalid encoding");

        entry->li = li;
        if (li->encoding == REDIS_ENCODING_ZIPLIST) {
                entry->zi = li->zi;
                if (entry->zi != NULL) {
                        if (li->direction == REDIS_TAIL)
                                li->zi = ziplistNext(li->subject->ptr,li->zi);
                        else
                                li->zi = ziplistPrev(li->subject->ptr,li->zi);
                        return 1;
                }
        } else if (li->encoding == REDIS_ENCODING_LINKEDLIST) {
                entry->ln = li->ln;
                if (entry->ln != NULL) {
                        if (li->direction == REDIS_TAIL)
                                li->ln = li->ln->next;
                        else
                                li->ln = li->ln->prev;
                        return 1;
                }
        } else {
                logicError("Unknown list encoding");
        }
        return 0;
}

/* Return entry or NULL at the current position of the iterator. */
robj *listTypeGet(listTypeEntry *entry) {
    listTypeIterator *li = entry->li;
    robj *value = NULL;
    if (li->encoding == REDIS_ENCODING_ZIPLIST) {
        unsigned char *vstr;
        unsigned int vlen;
        long long vlong;
        logicErrorExpr(entry->zi != NULL, "Never happend");
        if (ziplistGet(entry->zi,&vstr,&vlen,&vlong)) {
            if (vstr) {
                value = createStringObject((char*)vstr,vlen);
            } else {
                value = createStringObjectFromLongLong(vlong);
            }
        }
    } else if (li->encoding == REDIS_ENCODING_LINKEDLIST) {
            logicErrorExpr(entry->ln != NULL, "Never happend");
        value = listNodeValue(entry->ln);
        incrRefCount(value);
    } else {
        logicError("Unknown list encoding");
    }
    return value;
}

void listTypeInsert(listTypeEntry *entry, robj *value, int where) {
    robj *subject = entry->li->subject;
    if (entry->li->encoding == REDIS_ENCODING_ZIPLIST) {
        value = getDecodedObject(value);
        if (where == REDIS_TAIL) {
            unsigned char *next = ziplistNext(subject->ptr,entry->zi);

            /* When we insert after the current element, but the current element
             * is the tail of the list, we need to do a push. */
            if (next == NULL) {
                subject->ptr = ziplistPush(subject->ptr,value->ptr,sdslen(value->ptr),REDIS_TAIL);
            } else {
                subject->ptr = ziplistInsert(subject->ptr,next,value->ptr,sdslen(value->ptr));
            }
        } else {
            subject->ptr = ziplistInsert(subject->ptr,entry->zi,value->ptr,sdslen(value->ptr));
        }
        decrRefCount(value);
    } else if (entry->li->encoding == REDIS_ENCODING_LINKEDLIST) {
        if (where == REDIS_TAIL) {
            listInsertNode(subject->ptr,entry->ln,value,AL_START_TAIL);
        } else {
            listInsertNode(subject->ptr,entry->ln,value,AL_START_HEAD);
        }
        incrRefCount(value);
    } else {
        logicError("Unknown list encoding");
    }
}

/* Compare the given object with the entry at the current position. */
int listTypeEqual(listTypeEntry *entry, robj *o) {
    listTypeIterator *li = entry->li;
    if (li->encoding == REDIS_ENCODING_ZIPLIST) {
        logicErrorExpr(sdsEncodedObject(o), "Never happend");
        return ziplistCompare(entry->zi,o->ptr,sdslen(o->ptr));
    } else if (li->encoding == REDIS_ENCODING_LINKEDLIST) {
        return equalStringObjects(o,listNodeValue(entry->ln));
    } else {
            logicError("Unknown list encoding");
    }

    return 0;
}

/* Delete the element pointed to. */
void listTypeDelete(listTypeEntry *entry) {
    listTypeIterator *li = entry->li;
    if (li->encoding == REDIS_ENCODING_ZIPLIST) {
        unsigned char *p = entry->zi;
        li->subject->ptr = ziplistDelete(li->subject->ptr,&p);

        /* Update position of the iterator depending on the direction */
        if (li->direction == REDIS_TAIL)
            li->zi = p;
        else
            li->zi = ziplistPrev(li->subject->ptr,p);
    } else if (entry->li->encoding == REDIS_ENCODING_LINKEDLIST) {
        listNode *next;
        if (li->direction == REDIS_TAIL)
            next = entry->ln->next;
        else
            next = entry->ln->prev;
        listDelNode(li->subject->ptr,entry->ln);
        li->ln = next;
    } else {
        logicError("Unknown list encoding");
    }
}

void listTypeConvert(robj *subject, int enc) {
        listTypeIterator *li;
        listTypeEntry entry;
        logicErrorExpr(subject->type == REDIS_LIST, "Invalid type convert!");

        if (enc == REDIS_ENCODING_LINKEDLIST) {
                list *l = listCreate();
                listSetFreeMethod(l,decrRefCountVoid);

                /* listTypeGet returns a robj with incremented refcount */
                li = listTypeInitIterator(subject,0,REDIS_TAIL);
                while (listTypeNext(li,&entry)) listAddNodeTail(l,listTypeGet(&entry));
                listTypeReleaseIterator(li);

                subject->encoding = REDIS_ENCODING_LINKEDLIST;
                zfree(subject->ptr);
                subject->ptr = l;
        } else {
                logicError("Unsupported list conversion");
        }
}

/*-----------------------------------------------------------------------------
 * List Commands
 *----------------------------------------------------------------------------*/

void lpushCommand(caller_t* c)
{
    int i, pushed=0;
    unsigned long len;
    robj *lobj = lookupKeyWrite(c->db, c->argv[1]);
    robj *result;
    
    if (lobj && lobj->type != REDIS_LIST) {
        caller_set_err(c, ERR_TYPE);
        return ;
    }

    for (i = 2; i < c->argc; i++) {
        c->argv[i] = tryObjectEncoding(c->argv[i]);
        if (!lobj) {
            lobj = createZiplistObject();
            dbAdd(c->db, c->argv[1], lobj);
        }
        listTypePush(lobj, c->argv[i], REDIS_HEAD);
        pushed++;
    }
    
    len = lobj ? listTypeLength(lobj) : 0;

    /** prefer shared obj */
    if (len < REDIS_SHARED_INTEGERS) {
            result = shared.integers[len];
            incrRefCount(shared.integers[len]);
    } else {
            result = createObject(REDIS_STRING, (void*)(len));
            result->encoding = REDIS_ENCODING_INT;
    }
    caller_add_result(c, result);
    server.dirty += pushed;
}

void lrangeCommand(caller_t *c)
{
    robj *o;
    long long start, end;
    long llen, rangelen;

    if (getLongLongFromObject(c->argv[2], &start) != REDIS_OK ||
        getLongLongFromObject(c->argv[3], &end) != REDIS_OK)
    {
        caller_set_err(c,ERR_VALUE);
        return;
    }

    if ((o = lookupKeyRead(c->db,c->argv[1])) == NULL) {
        caller_set_err(c, ERR_NIL);
        return; 
    }

    if (o->type != REDIS_LIST) {
        caller_set_err(c, ERR_TYPE);
        return;
    }
    
    llen = listTypeLength(o);

    if (start < 0) start = llen+start;
    if (end < 0) end = llen+end;
    if (start < 0) start = 0;

    if (start > end || start >= llen) {
        caller_set_err(c, ERR_OUT_OF_RANGE);
        return;
    }

    if (end >= llen) end = llen-1;
    rangelen = (end-start)+1;

    if (o->encoding == REDIS_ENCODING_ZIPLIST) {
        unsigned char *p = ziplistIndex(o->ptr,start);
        unsigned char *vstr;
        unsigned int vlen;
        long long vlong;
        robj *result;

        while(rangelen--) {
            ziplistGet(p,&vstr,&vlen,&vlong);
            if (vstr) {
                    result = createObject(REDIS_STRING, sdsnewlen(vstr, vlen));
            } else {
                    if (vlong >= 0 && vlong < REDIS_SHARED_INTEGERS) {
                            result = shared.integers[vlong];
                            incrRefCount(shared.integers[vlong]);
                    } else {
                            result = createObject(REDIS_STRING, (void*)vlong);
                            result->encoding = REDIS_ENCODING_INT;
                    }
            }
            caller_add_result(c, result);
            p = ziplistNext(o->ptr,p);
        }
        
    } else if (o->encoding == REDIS_ENCODING_LINKEDLIST) {
        listNode *ln;
        
        if (start > llen/2) start -= llen;
        ln = listIndex(o->ptr,start);

        while(rangelen--) {
            caller_add_result(c, ln->value);
            incrRefCount(ln->value);
            ln = ln->next;
        }
    } else {
        logicError("List encoding is not LINKEDLIST nor ZIPLIST!");
    }
}

/*-----------------------------------------------------------------------------
 * Blocking POP operations
 *----------------------------------------------------------------------------*/

/* If the specified key has clients blocked waiting for list pushes, this
 * function will put the key reference into the server.ready_keys list.
 * Note that db->ready_keys is a hash table that allows us to avoid putting
 * the same key again and again in the list in case of multiple pushes
 * made by a script or in the context of MULTI/EXEC.
 *
 * The list will be finally processed by handleClientsBlockedOnLists() */
void signalListAsReady(redisDb *db, robj *key) {
    readyList *rl;

    /* No clients blocking for this key? No need to queue it. */
    if (dictFind(db->blocking_keys,key) == NULL) return;

    /* Key was already signaled? No need to queue it again. */
    if (dictFind(db->ready_keys,key) != NULL) return;

    /* Ok, we need to queue this key into server.ready_keys. */
    rl = zmalloc(sizeof(*rl));
    rl->key = key;
    rl->db = db;
    incrRefCount(key);
    listAddNodeTail(server.ready_keys,rl);

    /* We also add the key in the db->ready_keys dictionary in order
     * to avoid adding it multiple times into a list with a simple O(1)
     * check. */
    incrRefCount(key);
    logicErrorExpr(dictAdd(db->ready_keys,key,NULL) == DICT_OK, "Never happend");
}
