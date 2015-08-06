# CFLAGS=-pipe -std=gnu99 -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g -ggdb3 
#CFLAGS=-pipe -std=gnu99 -W -Wall -Wpointer-arith -Wno-unused-parameter 
CFLAGS = -shared -fPIC
ALL_INCS = -I /usr/local/include/luajit-2.0/ -I /usr/include/
ALL_LIBS = -lluajit-5.1 -luuid


cutils.so:
	gcc $(CFLAGS) cutils.c -o $@ $(ALL_INCS) $(ALL_LIBS) 
clean:
	rm  -rf *.so
