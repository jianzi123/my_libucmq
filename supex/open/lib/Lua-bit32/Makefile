NAME = lua-bit32
VERSION = 0.1
DIST := $(NAME)-$(VERSION)

CC = gcc
RM = rm -rf

CFLAGS = -Wall -g -fPIC -I/usr/local/include/luajit-2.0/
LFLAGS = -shared -L/usr/local/lib -lluajit-5.1
INSTALL_PATH = /usr/local/lib

all: bit32.so

bit32.so: bit32.o
	$(CC) -o $@ $< $(LFLAGS)

bit32.o: lbit32lib.c
	$(CC) -o $@ $(CFLAGS) -c $<

install: bit32.so
	install -D -s $< $(INSTALL_PATH)/$<

clean:
	$(RM) *.so *.o

dist:
	if [ -d $(DIST) ]; then $(RM) $(DIST); fi
	mkdir -p $(DIST)
	cp *.c Makefile $(DIST)/
	tar czvf $(DIST).tar.gz $(DIST)
	$(RM) $(DIST)

.PHONY: all clean dist
