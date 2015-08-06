
all: cJSON.o
	gcc -Wall -shared -fPIC $^ -o libcjson.so -lm
	ar -rcs libcjson.a $^

test:test.c
	gcc cJSON.c test.c -o test -lm
testjson:testjson.c
	gcc cJSON.c testjson.c -o testjson -lm

%.o : %.c
	gcc -fPIC -c $< -o $@

clean:
	rm -rf *.o *.a *.so

help:
	echo "get from http://sourceforge.net/projects/cjson/"
	echo "Last Update: 2013-08-14"
