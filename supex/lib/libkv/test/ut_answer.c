#include "ut_public.h"
#include <stdlib.h>



#define zmalloc malloc
#define zfree   free


#define ANSWER_HEAD 0
#define ANSWER_TAIL 1


typedef struct answer_value {
    void *ptr; // end with '\0'
    unsigned long ptrlen;
    struct answer_value* prev;
    struct answer_value* next;
}answer_value_t;
        
typedef struct answer {
    int errnum;
    char* err;
    unsigned long valuelen; 
    struct answer_value* head, *tail;
}answer_t;

typedef struct answer_iterator {
    answer_value_t *next;
    int direction;
}answer_iterator_t;


answer_t* answer_create()
{
    answer_t* r = zmalloc(sizeof(*r));
    if (r == NULL) return r;

//    r->errnum = ERR_NONE;
//    r->err = err_table[ERR_NONE>>8];
    r->valuelen = 0;
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
        zfree(del);
    }
    
    zfree(a);
}

answer_value_t *answer_value_create(const void *ptr, unsigned long ptrlen)
{
    answer_value_t* value;
    if (!ptr || ptrlen == 0) return NULL;

    value = zmalloc(sizeof(answer_value_t));
    value->ptr = zmalloc(ptrlen);
    memmove(value->ptr, ptr, ptrlen);
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
}

answer_iterator_t *answer_get_iter(answer_t *a, int direction)
{
    if (!a) return NULL;
    
    answer_iterator_t *iter = zmalloc(sizeof(answer_iterator_t));
    iter->direction = direction;
    if (iter->direction == ANSWER_HEAD) {
        iter->next = a->head;
    } else {
        iter->next = a->tail;
    }
}

void answer_rewind_iter(answer_t *a, answer_iterator_t *iter)
{
    if (iter->direction == ANSWER_HEAD) {
        iter->next = a->head;
    } else {
        iter->next = a->tail;
    }
}

void answer_release_iter(answer_iterator_t *iter)
{
    if (iter)
        zfree(iter);
}

answer_value_t *answer_next(answer_iterator_t *iter)
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


int start()
{
    printf("===> Begin Unit Test\n");
    return 0;
}

int end()
{
    printf("\n===> End Unit Test\n");
    return 0;
}

void test_answer()
{
    answer_value_t *value;
    answer_iterator_t *iter;
    
    char* datas[] =
    {
        "a",
        "b",
        "c"
    };
    
    answer_t *a = answer_create();
    CU_ASSERT(a != NULL);

    answer_add_value_tail(a, datas[0], strlen(datas[0]));
    answer_add_value_tail(a, datas[1], strlen(datas[1]));
    answer_add_value_tail(a, datas[2], strlen(datas[2]));

    iter = answer_get_iter(a, ANSWER_HEAD);
   
    while((value = answer_next(iter)) != NULL) {
        printf("%s\t", (char*)value->ptr);
    }
    puts("\n");
    
    answer_add_value_head(a, datas[0], strlen(datas[0]));
    answer_add_value_head(a, datas[1], strlen(datas[1]));
    answer_add_value_head(a, datas[2], strlen(datas[2]));
    answer_rewind_iter(a, iter);

    while((value = answer_next(iter)) != NULL) {
        printf("%s\t", (char*)value->ptr);
    }
    puts("\n");
     
    answer_release_iter(iter);
    answer_release(a);
}



int main(int argc, char *argv[])
{
    UTIL_INIT(start,end);
    UTIL_ADD_CASE("case 1:\n", test_answer);
    UTIL_RUN();
    UTIL_UNINIT();

    return 0;
}

