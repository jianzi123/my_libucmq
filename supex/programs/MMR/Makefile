BASE_SRC=./src
INC_DIR=-I/usr/local/include/luajit-2.0 -I../../lib/libev -I../../lib/json-c -I../../lib/http-parser -I../../lib/libcoro -I../../engine -I../../engine/major -I../../lib/liblrucache/src/ -I../../lib/libkv/release/ -I./src
LIB_DIR=-L/usr/local/lib -L../../lib -L../../engine/
LIBS=-lluajit-5.1 -lpthread -lrt -lm  
LIBA=-ljson-c -lstdc++

#[=========>MARK<=========]#
MAIN_APP_SERV ?= driview

#<--driview和appcenter需要连接lru库-->#
ifeq ($(MAIN_APP_SERV), $(filter $(MAIN_APP_SERV),appcenter driview))
LIBA += -llrucache -lkv
endif

ifeq ($(MAIN_APP_SERV), driview)
HANDLE_MODEL = LINE
PROTOCOL_STYLE = HTTP
endif

ifeq ($(MAIN_APP_SERV), appcenter)
HANDLE_MODEL = LINE
PROTOCOL_STYLE = HTTP
endif

ifeq ($(MAIN_APP_SERV), mdds)
HANDLE_MODEL = LINE
PROTOCOL_STYLE = HTTP
#<--mdds需要使用mysql和topoi-->#
INC_DIR += -I/usr/include/mysql -I../../lib/libtopoi -I ./ 
LIB_DIR += -L /usr/lib64/mysql 
LIBS += -lmysqlclient 	
LIBA += -ltopoi
endif
#<------------------------>#

-include $(HOME_PATH)/programs/link.mk
-include $(HOME_PATH)/share.mk

#<------mdds的宏--->#
ifeq ($(MAIN_APP_SERV), mdds)
ADD_CFLAGS += -DOPEN_MDDS
endif

CFLAGS +=$(ADD_CFLAGS)
CFLAGS +=$(EXPORT_CFLAGS)


#<------------------------>#
ifeq ($(HANDLE_MODEL),CORO)
ADD_OBJ += smart_coro_lua_api.o
endif
ifeq ($(HANDLE_MODEL), LINE)
ADD_OBJ += smart_line_lua_api.o
endif

#<--------------------------->#

BIN=./bin_$(MAIN_APP_SERV)

#<--driview 和appcenter需要使用的.o文件 -->#
ifeq ($(MAIN_APP_SERV), $(filter $(MAIN_APP_SERV),appcenter driview))
BASE_OBJ = $(addprefix $(BIN)/, \
           $(ADD_OBJ) \
           main.o \
           entry.o \
           lrucache.o \
	   libkv_cache.o \
           load_cfg.o \
           )
endif

#<-- mdds使用的.o文件 -->#
ifeq ($(MAIN_APP_SERV), mdds)
BASE_OBJ= $(addprefix $(BIN)/,\
	  $(ADD_OBJ) \
	  main.o \
	  get_poi.o \
          entry.o \
          load_cfg.o \
          )
endif


OBJ ?= $(BASE_OBJ)
SRV := $(MAIN_APP_SERV)
JOB += $(SRV)



all:$(JOB)
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)


$(SRV):$(OBJ)
	gcc $(CFLAGS) $^ $(INC_DIR) $(LIB_DIR) $(LIBA) $(LIBS) -o $@ 
	@-if [ ! -d ./logs ];then mkdir logs; fi
	@sh init/init.sh $@
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)


$(BIN)/%.o : $(BASE_SRC)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

$(BIN)/lrucache.o : ../../engine/lrucache.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@
$(BIN)/libkv_cache.o : ../../engine/libkv_cache.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

load:
ifeq ($(MMR), default)
	@echo -e $(RED)"ERROR operate!!!\n"$(GREEN)"please use like:"
	@echo -e $(YELLOW)$(foreach obj,$(SRV),"\t-->>make load MMR="$(obj)"\n")$(NONE)
else
	./$(MMR) -c $(MMR)_conf.json &
	sleep 10
	luajit init/$(MMR)_load.lua
	sleep 2
	killall $(MMR)
endif
push:
	git push origin HEAD:refs/for/master

clean:
	rm -rf ./bin_driview
	rm -rf ./bin_appcenter
	rm -rf ./bin_mdds
	rm -rf driview appcenter mdds
	@-if [ ! -d ./logs ];then mkdir logs; fi

distclean:
	make clean
	rm -rf ./logs/*.log
	rm -rf ./lua/driview/data/
	rm -rf ./lua/appcenter/data/
	rm -rf ./lua/mdds/data/

