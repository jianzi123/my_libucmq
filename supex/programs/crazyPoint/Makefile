BASE_SRC=./src
INC_DIR=-I/usr/local/include/luajit-2.0 -I../../lib/libev -I../../lib/json-c -I../../lib/http-parser \
	-I../../lib/leveldb-1.15.0/include/ -I../../lib/libcoro -I../../engine -I../../engine/major -I../../engine/minor -I./src
LIB_DIR=-L/usr/local/lib -L../../lib -L../../engine/
LIBS=-lluajit-5.1 -lpthread -lrt -lm -lstdc++
LIBA=-ljson-c -lleveldb -lsnappy

#[=========>MARK<=========]#
MAIN_APP_SERV ?= crzptY

ifeq ($(MAIN_APP_SERV), crzptX)
HANDLE_MODEL = LINE
PROTOCOL_STYLE = HTTP
endif
ifeq ($(MAIN_APP_SERV), crzptY)
HANDLE_MODEL = LINE
PROTOCOL_STYLE = HTTP
endif
#<------------------------>#
-include $(HOME_PATH)/programs/link.mk
-include $(HOME_PATH)/share.mk
CFLAGS +=$(ADD_CFLAGS)
CFLAGS +=$(EXPORT_CFLAGS)
#<------------------------>#

ADD_OBJ += crzpt_line_lua_api.o

#<--------------------------->#

BIN=./bin_$(MAIN_APP_SERV)
BASE_OBJ = $(addprefix $(BIN)/, \
	   $(ADD_OBJ) \
	   ldb.o \
	   main.o \
	   load_crzpt_cfg.o \
	   load_swift_cfg.o \
	   swift_lua_api.o \
	   )

OBJ ?= $(BASE_OBJ)
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

$(BIN)/ldb.o : ../../engine/ldb.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

push:
	git push origin HEAD:refs/for/master

clean:
	rm -rf -- -DATA
	rm -rf -- -COMD
	rm -rf ./bin_crzptX
	rm -rf ./bin_crzptY
	rm -rf crzptX crzptY
	@-if [ ! -d ./logs ];then mkdir logs; fi

distclean:
	make clean
	rm -rf ./logs/*.log
	rm -rf crzptDB


run:
	./crzptY

client:
	luajit test/crzptY_client.lua 
