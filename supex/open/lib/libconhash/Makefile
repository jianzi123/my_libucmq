
# output path
BIN = ./bin

# debug stuff 
ifeq ($(CFLAG), DEBUG)
CFLAGS += -g
endif
CFLAGS += -g

AR  = ar -cqs
# itermidiate objects
OBJ = $(addprefix $(BIN)/, \
	  md5.o \
	  util_rbtree.o  \
	  conhash_inter.o \
	  conhash_util.o  \
	  conhash.o \
	  )

SAMPLE_OBJS = $(addprefix $(BIN)/, \
		sample.o \
		)

LUA_CONHASH_OBJS = $(addprefix $(BIN)/, \
		lua_conhash.o \
		)
		
# include file path
INC = -I.
INC_LUA = -I. -I/usr/local/include/luajit-2.0

TARGETS = $(BIN)/libconhash.a $(BIN)/sample $(BIN)/conhash.so
 
all : clean prepare $(TARGETS)

# build libconhash as a static lib 
$(BIN)/libconhash.a : $(OBJ) 
	$(AR) $@ $(OBJ)
	
# build sample
$(BIN)/sample : $(SAMPLE_OBJS)
	gcc -O -o $@ $(SAMPLE_OBJS) -L. -L./bin -lconhash

$(BIN)/conhash.so : $(OBJ) $(LUA_CONHASH_OBJS)
	gcc -fPIC -shared -O -o $@ $(LUA_CONHASH_OBJS) $(OBJ)
	
$(BIN)/%.o : %.c
	gcc -fPIC $(INC) $(CFLAGS) -c $< -o $@	
$(BIN)/lua_conhash.o : lua_conhash.c
	gcc -fPIC $(INC_LUA) $(CFLAGS) -c $< -o $@	

# prepare the bin dir	
.PHONY : prepare	
prepare : 
		-mkdir $(BIN)
	  
.PHONY : clean
clean  :
		-rm -rf $(BIN)
