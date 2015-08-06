#[=========>MARK<=========]#
OBJECT_SCENE ?= ONLINE

#[=========>MARK<=========]#
HANDLE_MODEL ?= CORO

ifeq ($(HANDLE_MODEL), CORO)
ADD_CFLAGS += $(CORO_STACK_TYPE)
endif

ADD_CFLAGS += -DOPEN_$(HANDLE_MODEL)

#[=========>MARK<=========]#
PROTOCOL_STYLE ?= REDIS

ADD_CFLAGS += -DUSE_$(PROTOCOL_STYLE)_PROTOCOL




NONE            = "\x1B[m"
GRAY            = "\x1B[0;30m"
LIGHT_GRAY      = "\x1B[1;30m"
RED             = "\x1B[0;31m"
LIGHT_RED       = "\x1B[1;31m"
GREEN           = "\x1B[0;32m"
LIGHT_GREEN     = "\x1B[1;32m"
YELLOW          = "\x1B[0;33m"
LIGHT_YELLOW    = "\x1B[1;33m"
BLUE            = "\x1B[0;34m"
LIGHT_BLUE      = "\x1B[1;34m"
PURPLE          = "\x1B[0;35m"
LIGHT_PURPLE    = "\x1B[1;35m"
CYAN            = "\x1B[0;36m"
LIGHT_CYAN      = "\x1B[1;36m"
WHITE           = "\x1B[0;37m"
LIGHT_WHITE     = "\x1B[1;37m"
