BASE_SRC=./src
WITH_SRC=./cpp
INC_DIR=-I/usr/include/mysql -I/usr/local/include/luajit-2.0 -I../../lib/libev -I../../lib/json-c -I../../lib/libcoro -I../../lib/libtopo -I../../engine -I../../engine/major -I./cpp -I./src
LIB_DIR=-L/usr/local/lib -L/usr/lib64/mysql -L../../lib -L../../engine/
LIBS=-lmysqlclient -lluajit-5.1 -lpthread -lrt -lm
LIBA=-ljson-c -ltopo

#[=========>MARK<=========]#
MAIN_APP_SERV ?= topology

ifeq ($(MAIN_APP_SERV), topology)
HANDLE_MODEL = LINE
PROTOCOL_STYLE = REDIS
endif

#<------------------------>#
-include $(HOME_PATH)/programs/link.mk
-include $(HOME_PATH)/share.mk
CFLAGS +=$(ADD_CFLAGS)
CFLAGS +=$(EXPORT_CFLAGS)
#<------------------------>#

ADD_OBJ += smart_line_cpp_api.o

#<--------------------------->#

BIN=./bin_$(MAIN_APP_SERV)
BASE_OBJ = $(addprefix $(BIN)/, \
	   $(ADD_OBJ) \
	   main.o \
	   load_cfg.o \
	   )

WITH_OBJ = $(addprefix $(BIN)/, \
	   entry.o \
	   )

OBJ ?= $(BASE_OBJ) $(WITH_OBJ)
SRV := $(MAIN_APP_SERV)
JOB += $(SRV)



all:$(JOB)
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)


$(SRV):$(OBJ)
	gcc $(CFLAGS) $^ $(INC_DIR) $(LIB_DIR) $(LIBA) $(LIBS) -o $@
	@-if [ ! -d ./logs ];then mkdir logs; fi
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)


$(BIN)/%.o : $(BASE_SRC)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@


$(BIN)/%.o : $(WITH_SRC)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

push:
	git push origin HEAD:refs/for/master

clean:
	rm -rf ./bin_topology
	rm -rf topology
	@-if [ ! -d ./logs ];then mkdir logs; fi

distclean:
	make clean
	rm -rf ./logs/*.log

