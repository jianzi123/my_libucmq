BIN=./obj
SRC=./
INC_DIR=-I../json-c -I/usr/include/mysql
LIB_DIR=-L/usr/local/lib -L../ -L/usr/lib64/mysql
LIBS=-lrt -lm -lz -lmysqlclient
LIBA=#-ljson-c

JOB = graphdb test_memory libtopo.a

CFLAGS +=$(EXPORT_CFLAGS)
CFLAGS ?=-Wall -g

OBJ = $(addprefix $(BIN)/, \
      	util_map.o \
	memory_bath.o \
	sql_api.o \
	topo.o \
	topo_cfg.o \
	topo_api.o \
      )


all:$(JOB)
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)

libtopo.a:$(OBJ)
	ar -rcs $@ $^
	#gcc $(CFLAGS) $^ $(INC_DIR) $(LIB_DIR) $(LIBA) $(LIBS) -o $@

graphdb:
	gcc $(CFLAGS) -DTEST_ONLY sample.c memory_bath.c util_map.c topo.c topo_api.c sql_api.c $(INC_DIR) $(LIB_DIR) $(LIBS) -o $@
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

test_memory:
	gcc memory_bath.c test_memory_bath.c -o $@


$(BIN)/%.o:$(SRC)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@


clean:
	rm -rf $(BIN)
	rm -rf $(JOB)

distclean:
	make clean
