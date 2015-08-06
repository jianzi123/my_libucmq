BASE_SRC=./
WITH_SRC_MAJOR=./major
WITH_SRC_MINOR=./minor
INC_DIR=-I/usr/local/include/luajit-2.0 -I../lib/libev -I../lib/http-parser -I../lib/libcoro -I./major -I./minor -I.  -I../lib/liblrucache/src -I../lib/libkv/release
LIB_DIR=-L/usr/local/lib -L../lib
LIBS=-lluajit-5.1 -lpthread -lrt -lm -llrucache  -lkv
LIBA=#-lev -lcoro


#MAIN_SUPEX = supex_coro_http
#[=========>MARK<=========]#
ifeq ($(MAIN_SUPEX), supex_coro_redis)
HANDLE_MODEL = CORO
PROTOCOL_STYLE = REDIS
endif
ifeq ($(MAIN_SUPEX), supex_coro_http)
HANDLE_MODEL = CORO
PROTOCOL_STYLE = HTTP
endif
ifeq ($(MAIN_SUPEX), supex_line_redis)
HANDLE_MODEL = LINE
PROTOCOL_STYLE = REDIS
endif
ifeq ($(MAIN_SUPEX), supex_line_http)
HANDLE_MODEL = LINE
PROTOCOL_STYLE = HTTP
endif
ifeq ($(MAIN_SUPEX), supex_evuv_redis)
HANDLE_MODEL = EVUV
PROTOCOL_STYLE = REDIS
endif
ifeq ($(MAIN_SUPEX), supex_evuv_http)
HANDLE_MODEL = EVUV
PROTOCOL_STYLE = HTTP
endif
#<------------------------>#
include $(HOME_PATH)/share.mk
CFLAGS +=$(ADD_CFLAGS)
CFLAGS +=$(EXPORT_CFLAGS)
#<------------------------>#


#[=========>MARK<=========]#
ifeq ($(PROTOCOL_STYLE), HTTP)
ADD_OBJ += http.o
#ADD_OBJ += http_parser.o
endif
ifeq ($(PROTOCOL_STYLE), REDIS)
ADD_OBJ += redis_parser.o
endif

#[=========>MARK<=========]#
ifeq ($(OBJECT_SCENE), OFFLINE)
ADD_OBJ += instrument.o
endif

#<------------------------>#
BIN=./bin_$(MAIN_SUPEX)
BASE_OBJ = $(addprefix $(BIN)/, \
	   $(ADD_OBJ) \
	   net_cache.o \
	   hashmap.o \
	   sstr.o \
	   cnt_pool.o \
	   pool_api.o \
	   async_api.o \
	   async_libev.o \
	   http_client.o \
	   http_parser.o \
	   major_def.o \
	   share_evcb.o \
	   smart_task.o \
	   smart_evcb.o \
	   smart_api.o \
	   swift_api.o \
	   swift_evcb.o \
	   swift_task.o \
	   crzpt_task.o \
	   crzpt_plan.o \
	   crzpt_evcb.o \
	   crzpt_api.o \
	   sniff_api.o \
	   sniff_task.o \
	   coro.o \
	   supex.o \
	   tcp_api.o \
	   sync_link.o \
	   async_link.o \
	   list.o  \
	   utils.o \
	   match.o \
	   )

OBJ ?= $(BASE_OBJ)
PJT := lib$(MAIN_SUPEX).a
JOB += $(PJT) xctl



all:$(JOB)
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)


$(PJT):$(OBJ)
	cd $(BIN) && ar -x ../../lib/libev.a
	ar -rcs $@ $(BIN)/*.o
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

$(BIN)/instrument.o : ../pvtrace/instrument.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

$(BIN)/http_parser.o : ../lib/http-parser/http_parser.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

$(BIN)/coro.o : ../lib/libcoro/coro.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

$(BIN)/%.o : $(BASE_SRC)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

$(BIN)/%.o : $(WITH_SRC_MAJOR)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

$(BIN)/%.o : $(WITH_SRC_MINOR)/%.c
	@-if [ ! -d $(BIN) ];then mkdir $(BIN); fi
	gcc $(CFLAGS) $(INC_DIR) $(LIB_DIR) $(LIBS) -c $< -o $@

xctl:
	gcc $(BASE_SRC)/xctl.c -lrt -lluajit-5.1 $(INC_DIR) -o $@
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

clean:
	rm -rf $(BIN)
	rm -rf $(PJT)
	rm -rf ./xctl

distclean:
	rm -rf ./bin_supex_coro_redis
	rm -rf ./bin_supex_coro_http
	rm -rf ./bin_supex_line_redis
	rm -rf ./bin_supex_line_http
	rm -rf ./bin_supex_evuv_redis
	rm -rf ./bin_supex_evuv_http
	rm -rf ./libsupex_*.a
	rm -rf ./xctl
