JSON_PATH=../../../lib/cJSON
JSON_SOUCE=$(JSON_PATH)/cJSON.c

all:
	gcc -Wall tcpsrv.c wrap.c $(JSON_SOUCE) -fPIC -shared -I/usr/local/include/luajit-2.0  -I$(JSON_PATH) -o tcpsrv.so -lpthread -lm

