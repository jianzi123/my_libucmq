/**
 * @brief Unit test for converting redis protocol to reply by shishengjie
 */

#include <ctype.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include "test_redis_reply.h"




#define BOOL_TRUE  1
#define BOOL_FALSE 0

#define OK  BOOL_TRUE
#define ERROR -1


typedef enum status {
        STATUS_CLOSED = 0x00,
        STATUS_CONNECTING = 0x01,
        STATUS_CONNECTED = 0x02,
        STATUS_ESTABLISHED = 0x04
}status_e;

typedef struct front_weibo {
        int sfd;
        struct sockaddr_in serv_addr;
        status_e status;

}front_weibo_t;


front_weibo_t *fweibo;


int cmd_to_proto(char **proto, const char *fmt, ...);


void sig_handler(int sig)
{
	if (fweibo->sfd > 0)
		close(fweibo->sfd);	
}



front_weibo_t* front_weibo_create()
{
        front_weibo_t *fweibo;

        fweibo = calloc(1, sizeof(front_weibo_t));
        assert(fweibo);

        fweibo->sfd = -1;

        return fweibo;
}

void front_weibo_release(front_weibo_t *fweibo)
{
        free(fweibo);
}

int _socket(front_weibo_t *fweibo)
{
        int ret;
        struct addrinfo hints, *cur, *res;

        bzero(&hints, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        
        ret = getaddrinfo("127.0.0.1", NULL, &hints, &res);
        assert(ret == 0);

        for (cur = res; cur; cur = cur->ai_next) {
                fweibo->sfd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
                if (fweibo->sfd == -1) continue;
                else break;
        }
        if (cur == NULL)
                printf("No address resource?\n"), assert(0);

        freeaddrinfo(res);
        return OK;
}

int _close(front_weibo_t *fweibo)
{

        return ERROR;
}

void _human_to_netaddr(struct sockaddr_in *netaddr, short port, const char *addr)
{
        int ret;
        
        bzero(netaddr, sizeof(*netaddr));
        netaddr->sin_family = AF_INET;
        netaddr->sin_port = htons(port);
        ret = inet_pton(netaddr->sin_family, addr, &netaddr->sin_addr.s_addr);
        switch(ret) {
        case 0:
                printf ("Invalid address in domain"), abort();
        case -1:
                printf ("Invalid domain"), abort();
        }
}

int front_weibo_connect(front_weibo_t *fweibo, const char *addr, short port)
{
        int ret;
        socklen_t len = sizeof(struct sockaddr_in);
        
        if (fweibo->status & STATUS_CONNECTED)
                return EISCONN;
        if (fweibo->status & STATUS_CONNECTING)
                return EINPROGRESS;

        _socket(fweibo);
        _human_to_netaddr(&fweibo->serv_addr, port, addr);
        fweibo->status = STATUS_CONNECTING;
        ret = connect(fweibo->sfd, (const struct sockaddr*)(&(fweibo->serv_addr)), len);
        if (ret != 0) {
                perror("Connect failed->");
                abort();
                exit(1);
        }
        fweibo->status = STATUS_CONNECTED;
        
        return OK;
}

int front_weibo_disconnect(front_weibo_t *fweibo)
{
        if (fweibo->status & STATUS_CONNECTED) {
                close(fweibo->sfd);
                fweibo->sfd = -1;
                fweibo->status = STATUS_CLOSED;
                return OK;
        }
        return ERROR;
}

size_t front_weibo_send(front_weibo_t *fweibo, const char *buf, size_t buflen)
{
        return send(fweibo->sfd, buf, buflen, 0);
}

size_t front_weibo_recv(front_weibo_t *fweibo, char *buf, size_t buflen)
{
        return recv(fweibo->sfd, buf, buflen, 0);
}








#ifdef TEST_PROTO_TO_REPLY

int ret;
char *proto;
front_weibo_t *fweibo;
char case_buf[1024*16];
size_t send_bytes, recv_bytes;  


void _proto_print(const char *proto, size_t len)
{
        int i = 0;

        assert(proto);
        for (; i < len; i++) {
                switch(proto[i]) {
                case '\r':
                        printf("\\r");
                        break;
                case '\n':
                        printf("\\n");
                        break;
                default:
                        printf("%c", proto[i]);
                }
        }
        puts("\n");
}

/**
 * return protocol length.
 * protocol string stores in case_buf.
 */
size_t _get_reply_proto(const char *proto, size_t protolen)
{
        send_bytes = front_weibo_send(fweibo, proto, protolen);
        assert(send_bytes == protolen);

        return front_weibo_recv(fweibo, case_buf, sizeof(case_buf));
}

void _reply_unpack(const struct redis_reply *reply)
{
        assert(reply);
        
        switch(reply->type) {
        case REDIS_REPLY_STRING:
                printf("===>reply type string:%s\n", reply->str);
                break;
        case REDIS_REPLY_INTEGER:
                printf("===>reply type integer:%lld\n", reply->integer);
                break;
        case REDIS_REPLY_NIL:
                printf("===>reply type nil\n");
                break;
        case REDIS_REPLY_STATUS:
                printf("===>reply type status:%s\n", reply->str);
                break;
        case REDIS_REPLY_ERROR:
                printf("===>reply type error:%s\n", reply->str);
                break;
        case REDIS_REPLY_ARRAY:
                printf("===>reply type array:\n");
                printf("\t===>reply->elements:%zu\n", reply->elements);
                int i;
                for (i = 0; i < reply->elements; i++) {
                        printf("\t===>reply->element[%d]->elements:%zu\tstr:%s len:%d\n",
                               i, reply->element[i]->elements, reply->element[i]->str, reply->element[i]->len);
                }
                printf("<===reply type array\n");
                redis_reply_release((struct redis_reply*)reply);
                break;
        default:
                printf("===>unknown type\n");
        }
}

void case_invalid_cmd()
{
        size_t bytes;
        size_t reply_len;
        struct redis_reply *reply;
        
        cmd_to_proto(&proto, "set_invalid key value");
        bytes = _get_reply_proto(proto, strlen(proto));
        free(proto);
        
//        _proto_print(case_buf, bytes);

        ret = first_reply_ok(case_buf, bytes, &reply_len);
        if (!ret) { 
                printf("===>No first reply available\n");
                return;
        }

        reply = proto_to_reply(case_buf, bytes);
        _reply_unpack(reply);  
}

void case_OK()
{
        size_t bytes;
        size_t reply_len;
        struct redis_reply *reply;
        
        /**
         * set key value
         * +OK\r\n
         */
        cmd_to_proto(&proto, "set key value");
        bytes = _get_reply_proto(proto, strlen(proto));
        free(proto);
        
        _proto_print(case_buf, bytes);
        assert(memcmp(case_buf, "+OK\r\n", bytes) == 0);

        ret = first_reply_ok(case_buf, bytes, &reply_len);
        if (!ret) { 
                printf("===>No first reply available\n");
                return;
        }

        reply = proto_to_reply(case_buf, bytes);
        _reply_unpack(reply);
}

void case_nil()
{
        size_t bytes;
        size_t reply_len;
        struct redis_reply *reply;
        
        /**
         * get no-exist
         * $-1\r\n
         */
        cmd_to_proto(&proto, "get no-exist");
        bytes = _get_reply_proto(proto, strlen(proto));
        free(proto);
        
        _proto_print(case_buf, bytes);
        assert(memcmp(case_buf, "$-1\r\n", bytes) == 0);

        ret = first_reply_ok(case_buf, bytes, &reply_len);
        if (!ret) { 
                printf("===>No first reply available\n");
                return;
        }
        reply = proto_to_reply(case_buf, bytes);
        _reply_unpack(reply);
}

void case_string()
{
        size_t bytes;
        size_t reply_len;
        struct redis_reply *reply;
        
        /**
         * get key
         * $value\r\n
         */
        cmd_to_proto(&proto, "get key");
        bytes = _get_reply_proto(proto, strlen(proto));
        free(proto);
        
        _proto_print(case_buf, bytes);
//        assert(memcmp(case_buf, "$-1\r\n", bytes) == 0);

        ret = first_reply_ok(case_buf, bytes, &reply_len);
        if (!ret) { 
                printf("===>No first reply available\n");
                return;
        }
        reply = proto_to_reply(case_buf, bytes);
        _reply_unpack(reply);
}

void case_number()
{
        size_t bytes;
        size_t reply_len;
        struct redis_reply *reply;
        
        /**
         * exist key
         * :1\r\n
         */
        cmd_to_proto(&proto, "exists key");
        bytes = _get_reply_proto(proto, strlen(proto));
        free(proto);
        
        _proto_print(case_buf, bytes);

        ret = first_reply_ok(case_buf, bytes, &reply_len);
        if (!ret) { 
                printf("===>No first reply available\n");
                return;
        }
        reply = proto_to_reply(case_buf, bytes);
        _reply_unpack(reply);
}

void case_array()
{
        size_t bytes;
        size_t reply_len;
        struct redis_reply *reply;
        
        /**
         * lpush lkey a bb ccc dddd
         * lrange lkey 0 -1
         * *4\r\n$4\r\ndddd\r\n$3\r\nccc\r\n$2\r\nbb\r\n$1\r\na\r\n
         */
        cmd_to_proto(&proto, "lrange lkey 0 -1");
        bytes = _get_reply_proto(proto, strlen(proto));
        free(proto);
        
        _proto_print(case_buf, bytes);
        
        ret = first_reply_ok(case_buf, bytes, &reply_len);
        if (!ret) { 
                printf("===>No first reply available\n");
                return;
        }
        reply = proto_to_reply(case_buf, bytes);
        _reply_unpack(reply);
}


int main(int argc, char *argv[])
{
        signal(SIGTERM, sig_handler);
        fweibo = front_weibo_create();
        front_weibo_connect(fweibo, "127.0.0.1", 6379);

        //case_invalid_cmd();
        //case_OK();
        //case_nil();
        //case_number();
        //case_string();
        case_array();
        
        front_weibo_disconnect(fweibo);

        return 0;
}

#endif
