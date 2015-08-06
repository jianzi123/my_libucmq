#
# Makefile for the trace utility.
#
# M. Tim Jones <mtj@mtjones.com>
#

CC = gcc

OBJS = trace.o symbols.o stack.o

pvtrace: $(OBJS)
	gcc -o $@ $(OBJS)

test:
	gcc -g -finstrument-functions test.c instrument.c -o test
	./test

view:
	./pvtrace test
	dot -Tpng graph.dot -o graph.png
	eog -f graph.png

.c.o:
	$(CC) $(CFLAGS) -Wall -c $<

install: pvtrace
	cp pvtrace /usr/local/bin

clean:
	rm -f pvtrace test trace.txt graph.dot graph.png *.o
