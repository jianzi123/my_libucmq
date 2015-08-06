BASE_SRC=./src
INC_DIR=-I/usr/include/mysql -I/usr/local/include/luajit-2.0 -I../../lib/libev -I../../lib/json-c -I../../lib/http-parser -I../../lib/libcoro -I../../engine -I../../engine/major -I./src -I../../lib/libtopoi -I ./
LIB_DIR=-L/usr/local/lib -L../../lib -L../../engine/ -L /usr/lib64/mysql 
LIBS=-lluajit-5.1 -lpthread -lrt -lm -lmysqlclient
LIBA=-ljson-c -ltopoi  

#[=========>MARK<=========]#
MAIN_APP_SERV ?= mdds

ifeq ($(MAIN_APP_SERV), mdds)
HANDLE_MODEL = LINE
PROTOCOL_STYLE = HTTP
endif
#<------------------------>#
-include $(HOME_PATH)/programs/link.mk
-include $(HOME_PATH)/share.mk
CFLAGS +=$(ADD_CFLAGS)
CFLAGS +=$(EXPORT_CFLAGS)
#<------------------------>#

ADD_OBJ += smart_line_lua_api.o

#<--------------------------->#

BIN=./bin_$(MAIN_APP_SERV)
BASE_OBJ = $(addprefix $(BIN)/, \
	   $(ADD_OBJ) \
	   main.o \
	   get_poi.o \
	   entry.o \
	   load_cfg.o \
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

push:
	git push origin HEAD:refs/for/master

clean:
	rm -rf ./bin_mdds
	rm -rf mdds
	@-if [ ! -d ./logs ];then mkdir logs; fi

distclean:
	make clean
	rm -rf ./logs/*.log

