#
# date: 2014-10-18
#

TARGET=libldb.so
TEST=ldb_test
OBJ_DIR=./obj
SRC=./src
INC_DIR=-I../include \
	-I/usr/local/include/luajit-2.0/ \
	-I/usr/include/

LIB_DIR=../../

TEST_DEPS = $(addprefix $(OBJ_DIR)/, ldb.o main.o)
TARGET_DEPS = $(addprefix $(OBJ_DIR)/, ldb.o ldb_lua_api.o)

CFLAGS=-Wall -O2 -DOPEN_COMPRESSION -fPIC -g
DFLAGS=-Wall -O2 -DOPEN_COMPRESSION -fPIC -g -shared

LIBS=-lpthread -lstdc++ -lm -lluajit-5.1
LIBA=-lleveldb -lsnappy

help:
	@echo "make libs first!"
libs:
	@echo "TODO..."

all:$(TEST_DEPS) $(TARGET_DEPS)
	gcc $(CFLAGS) $(TEST_DEPS) $(INC_DIR) -L$(LIB_DIR) $(LIBA) $(LIBS) -o $(TEST)
	gcc $(DFLAGS) $(TARGET_DEPS) $(INC_DIR) -L$(LIB_DIR) -L../ -L../../snappy-1.1.1/.libs $(LIBA) $(LIBS) -o $(TARGET)

$(OBJ_DIR)/%.o:$(SRC)/%.c
	@-if [ ! -d $(OBJ_DIR) ];then mkdir $(OBJ_DIR); fi
	gcc $(CFLAGS) $(INC_DIR) -L$(LIB_DIR) $(LIBS) -c $< -o $@

clean:
	rm -f $(TEST) $(TARGET)
	rm -rf $(OBJ_DIR)/*.o
	@-if [ ! -d $(OBJ_DIR) ];then mkdir $(OBJ_DIR); fi

distclean:
	$(MAKE) clean
	$(MAKE) -C $(LIB_DIR) clean
