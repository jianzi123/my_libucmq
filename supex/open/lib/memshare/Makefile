

all:
	gcc -Wall memshare.c -fPIC -shared -I/usr/local/include/luajit-2.0 -o memshare.so -lrt -lpthread -D_GNU_SOURCE

test:
	luajit test.lua

tests:
	luajit test1.lua
	luajit test2.lua
logs:
	luajit logs_test.lua

