OBJ = libcoro.a \
      sample


all:$(OBJ)
	-@echo "OK!"

sample:coro_example.o libcoro.a
	gcc $< -lcoro -L. -o $@

libcoro.a:coro.o
ifeq ($(CORO_STACK_TYPE),)
	@echo -e "\x1B[0;31m\t""don't make at libcoro directory!""\x1B[m"
else
	ar -rcs $@ $^
	nm -s $@ > /dev/null
endif

%.o:%.c
	#@if [ ! $(EXPORT_CFLAGS) ];then echo -e "\x1B[0;31m\t""don't make at here!""\x1B[m";else gcc $(EXPORT_CFLAGS) -c $< ; fi
	gcc $(EXPORT_CFLAGS) $(CORO_STACK_TYPE) -c $<

clean:
	rm -rf *.o $(OBJ)

