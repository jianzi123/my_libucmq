BASE_SRC=./src
INC_DIR=-I/usr/local/include/luajit-2.0 -I../../lib/libev -I../../lib/json-c -I../../lib/http-parser -I../../lib/libcoro -I../../engine -I../../engine/major -I./src
LIB_DIR=-L/usr/local/lib -L../../lib -L../../engine/
LIBS=-lluajit-5.1 -lpthread -lrt -lm
LIBA=-ljson-c

#[=========>MARK<=========]#
MAIN_APP_SERV ?= spray

ifeq ($(MAIN_APP_SERV), spray)
HANDLE_MODEL = LINE
PROTOCOL_STYLE = HTTP
ADD_OBJ += swift_lua_api.o spray_main.o
endif

ifeq ($(MAIN_APP_SERV), spark)
HANDLE_MODEL = LINE
PROTOCOL_STYLE = HTTP
ADD_OBJ += swift_cpp_api.o spark_main.o
endif

#<------------------------>#
-include $(HOME_PATH)/programs/link.mk
-include $(HOME_PATH)/share.mk
CFLAGS +=$(ADD_CFLAGS)
CFLAGS +=$(EXPORT_CFLAGS)
#<------------------------>#

LIBA += -lsupex_line_http

#<--------------------------->#

BIN=./bin_$(MAIN_APP_SERV)
BASE_OBJ = $(addprefix $(BIN)/, \
	   $(ADD_OBJ) \
	   load_swift_cfg.o \
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
	rm -rf ./bin_spray
	rm -rf ./bin_spark
	rm -rf spray spark
	@-if [ ! -d ./logs ];then mkdir logs; fi

distclean:
	make clean
	rm -rf ./logs/*.log

