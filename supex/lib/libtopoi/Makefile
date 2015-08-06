BIN=./obj
SRC1=./topo
SRC2=./vector
INC_DIR=-I../json-c -I/usr/include/mysql -I./ -I../json-c
LIB_DIR=-L/usr/local/lib -L../ -L/usr/lib64/mysql
LIBS=-lrt -lm -lz -lmysqlclient
LIBA=#-ljson-c

JOB = graphdbtopo graphdbvector test_memory libtopoi.a

CFLAGS +=$(EXPORT_CFLAGS)
CFLAGS ?=-Wall -g

OBJ = $(addprefix $(BIN)/, \
      	util_map.o \
	memory_bath.o \
	sql_api.o \
	topo.o \
	topo_cfg.o \
	topo_api.o \
	vector.o \
	vector_cfg.o \
	vector_api.o \
      )


all: $(JOB)
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)

libtopoi.a:$(OBJ)
	ar -rcs $@ $^
	#gcc $(CFLAGS) $^ $(INC_DIR) $(LIB_DIR) $(LIBA) $(LIBS) -o $@

graphdbtopo:
	gcc $(CFLAGS) -DTEST_ONLY topo/sample.c  memory_bath.c topo/util_map.c topo/topo.c topo/topo_api.c sql_api.c $(INC_DIR) $(LIB_DIR) $(LIBS) -o $@
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

graphdbvector:
	gcc $(CFLAGS)  -DTEST_ONLY vector/sample.c  memory_bath.c  vector/vector.c vector/vector_api.c sql_api.c $(INC_DIR) $(LIB_DIR) $(LIBS) -o $@
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

test_memory:
	gcc memory_bath.c test_memory_bath.c -o $@

$(BIN)/%.o: %.c 
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@
	
$(BIN)/%.o:$(SRC1)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

$(BIN)/%.o:$(SRC2)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@


clean:
	rm -rf $(BIN)
	rm -rf $(JOB)

distclean:
	make clean
