


TARGET_TEST_PROTO_TO_REPLY=test_proto_to_reply

TARGET_ALL=$(TARGET_TEST_PROTO_TO_REPLY)


all:
	gcc -o test_proto_to_reply test_proto_to_reply.c test_redis_reply.c test_sstr.c test_cmd_to_proto.c  -Wall -DTEST_PROTO_TO_REPLY



clean:
	rm -f *.o *.*~ $(TARGET_ALL)
