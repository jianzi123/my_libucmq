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
######################################
str_kernel_version = $(shell uname -r)
get_kernel_version = $(shell echo $1 | sed 's/\./ /g' | sed 's/-/ /g')
fix_kernel_version = $(call get_kernel_version, $(str_kernel_version))

v_major = $(word 1, $(fix_kernel_version))
v_minor = $(word 2, $(fix_kernel_version))
v_emend = $(word 3, $(fix_kernel_version))

OPTIMIZE = 0

compare = $(shell if [ $1 -eq $2 ]; then echo ?; elif [ $1 -gt $2 ]; then echo 1; else echo 0; fi)
OPTIMIZE = $(call compare, $(v_major), 2)

ifeq ($(OPTIMIZE), ?)
OPTIMIZE = $(call compare, $(v_minor), 6)
ifeq ($(OPTIMIZE), ?)
OPTIMIZE = $(call compare, $(v_emend), 35)
endif
endif
ifeq ($(OPTIMIZE), ?)
OPTIMIZE = 1
endif

ifeq ($(OPTIMIZE), 1)
EXCESS_CFLAGS += -DOPEN_OPTIMIZE
endif
######################################

export OBJECT_SCENE ?= ONLINE
export CORO_STACK_TYPE ?= -DCORO_USE_STATIC_STACK

export HOME_PATH = $(shell pwd)
EXPORT_CFLAGS = -g
ifeq ($(OBJECT_SCENE), ONLINE)
EXPORT_CFLAGS += -O1
else
EXPORT_CFLAGS += -finstrument-functions
endif
EXPORT_CFLAGS += $(EXCESS_CFLAGS)
export EXPORT_CFLAGS +=-Wall \
	 -DUSE_PIPE \
	 -DUSE_SPINLOCK \
	 -DCRZPT_OPEN_MSMQ \
	 -DCRZPT_MSMQ_SELECT_ASYNC \
	 #-DOPEN_DEBUG \
	 #-DCRZPT_MSMQ_SELECT_SYNC \
	 #-DUSE_MUTEX \
	 #-DOPEN_TIME_OUT \
	 #-static-libgcc -static-libstdc++

SRV := topology ptop spxapi robais driview appcenter crzptX crzptY mdds damS stsv ashman weibo weidb drimode rtmiles spray spark checkequipment

help:
	@echo -e $(GREEN) "make libs first!" $(NONE)
	@echo -e $(RED) "make " $(BLUE) $(foreach n,$(SRV),[$(PURPLE)$(n)$(BLUE)]) $(NONE)
	@echo -e $(GREEN) "For more details, please look!!!" $(NONE)

libs:
	$(MAKE) -C ./lib clean
	$(MAKE) -C ./lib
	sleep 1
	$(MAKE) -C ./engine clean MAIN_SUPEX=supex_coro_http
	$(MAKE) -C ./engine MAIN_SUPEX=supex_coro_http
	$(MAKE) -C ./engine clean MAIN_SUPEX=supex_line_http
	$(MAKE) -C ./engine MAIN_SUPEX=supex_line_http
	$(MAKE) -C ./engine clean MAIN_SUPEX=supex_evuv_http
	$(MAKE) -C ./engine MAIN_SUPEX=supex_evuv_http
	$(MAKE) -C ./engine clean MAIN_SUPEX=supex_coro_redis
	$(MAKE) -C ./engine MAIN_SUPEX=supex_coro_redis
	$(MAKE) -C ./engine clean MAIN_SUPEX=supex_line_redis
	$(MAKE) -C ./engine MAIN_SUPEX=supex_line_redis
	$(MAKE) -C ./engine clean MAIN_SUPEX=supex_evuv_redis
	$(MAKE) -C ./engine MAIN_SUPEX=supex_evuv_redis
	$(MAKE) -C ./engine xctl
	sleep 1
	$(MAKE) -C ./open/lib clean
	$(MAKE) -C ./open/lib/

all:$(SRV)
	@echo -e $(foreach bin,$^,$(BLUE)$(bin) $(GREEN)✔)$(RED)"\n-->OVER!\n"$(NONE)


ptop:
	$(MAKE) -C ./programs/RTAP MAIN_APP_SERV=ptop
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)
spxapi:
	$(MAKE) -C ./programs/RTAP MAIN_APP_SERV=spxapi
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)
robais:
	$(MAKE) -C ./programs/RTAP MAIN_APP_SERV=robais
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

driview:
	$(MAKE) -C ./programs/MMR MAIN_APP_SERV=driview
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)
appcenter:
	$(MAKE) -C ./programs/MMR MAIN_APP_SERV=appcenter
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

topology:
	$(MAKE) -C ./programs/roadRelation MAIN_APP_SERV=topology
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

crzptX:
	$(MAKE) -C ./programs/crazyPoint MAIN_APP_SERV=crzptX
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

crzptY:
	$(MAKE) -C ./programs/crazyPoint MAIN_APP_SERV=crzptY
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

mdds:
	$(MAKE) -C ./programs/MMR MAIN_APP_SERV=mdds
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

damS:
	$(MAKE) -C ./programs/greatWall MAIN_APP_SERV=damS
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

stsv:
	$(MAKE) -C ./programs/flyKite MAIN_APP_SERV=stsv
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

ashman:
	$(MAKE) -C ./programs/ashman MAIN_APP_SERV=ashman
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

weibo:
	$(MAKE) -C ./programs/weibo MAIN_APP_SERV=weibo
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

weidb:
	$(MAKE) -C ./programs/weidb MAIN_APP_SERV=weidb
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

drimode:
	$(MAKE) -C ./programs/drimode MAIN_APP_SERV=drimode
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

checkequipment:
	$(MAKE) -C ./programs/checkequipment MAIN_APP_SERV=checkequipment
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

rtmiles:
	$(MAKE) -C ./programs/rtmiles MAIN_APP_SERV=rtmiles
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

spray:
	$(MAKE) -C ./programs/crystal MAIN_APP_SERV=spray
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

spark:
	$(MAKE) -C ./programs/crystal MAIN_APP_SERV=spark
	@echo -e $(GREEN)"【"$(YELLOW) $@ $(GREEN)"】"$(RED)"\n-->OK!\n"$(NONE)

push:
	git push origin HEAD:refs/for/devel

clean:
	$(MAKE) -C ./programs/RTAP clean
	$(MAKE) -C ./programs/MMR clean
	$(MAKE) -C ./programs/roadRelation clean
	$(MAKE) -C ./programs/crazyPoint clean
	$(MAKE) -C ./programs/MMR clean
	$(MAKE) -C ./programs/greatWall clean
	$(MAKE) -C ./programs/flyKite clean
	$(MAKE) -C ./programs/ashman clean
	$(MAKE) -C ./programs/weibo clean
	$(MAKE) -C ./programs/weidb clean
	$(MAKE) -C ./programs/drimode clean
	$(MAKE) -C ./programs/checkequipment clean
	$(MAKE) -C ./programs/rtmiles clean
	$(MAKE) -C ./programs/crystal clean

distclean:
	$(MAKE) -C ./programs/RTAP distclean
	$(MAKE) -C ./programs/MMR distclean
	$(MAKE) -C ./programs/roadRelation distclean
	$(MAKE) -C ./programs/crazyPoint distclean
	$(MAKE) -C ./programs/MMR distclean
	$(MAKE) -C ./programs/greatWall distclean
	$(MAKE) -C ./programs/flyKite distclean
	$(MAKE) -C ./programs/ashman distclean
	$(MAKE) -C ./programs/weibo distclean
	$(MAKE) -C ./programs/weidb distclean
	$(MAKE) -C ./programs/drimode distclean
	$(MAKE) -C ./programs/checkequipment distclean
	$(MAKE) -C ./programs/rtmiles distclean
	$(MAKE) -C ./programs/crystal distclean
	$(MAKE) -C ./lib clean
	$(MAKE) -C ./open/lib clean
	$(MAKE) -C ./engine distclean

########################################DONT USE#####################################################
install:
	cd lib/libev/  && ./configure && make && sudo make install
	sudo ldconfig
	@echo -e $(RED) "     >OK<" $(NONE)

other:
	yum install autoconf
	yum install automake.noarch
	yum install libtool.x86_64
	yum install libuuid-devel
	#yum install gcc-c++.x86_64
	#yum install libstdc++-devel.x86_64
	# mkdir /dev/mqueue
	# mount -t mqueue none /dev/mqueue

link:
	ln -s /usr/local/lib/libev.so  /usr/lib64/libev.so
	ln -s /usr/local/lib/libev.so.4  /usr/lib64/libev.so.4
	#链接是用绝对路径
	@echo "     >OK<"

tag:
	ctags -R

cs:
	cscope -Rbq

