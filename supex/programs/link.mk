#[=========>MARK<=========]#
ifeq ($(HANDLE_MODEL), CORO)
ifeq ($(PROTOCOL_STYLE), REDIS)
LIBA += -lsupex_coro_redis
else
LIBA += -lsupex_coro_http
endif
endif

ifeq ($(HANDLE_MODEL), LINE)
ifeq ($(PROTOCOL_STYLE), REDIS)
LIBA += -lsupex_line_redis
else
LIBA += -lsupex_line_http
endif
endif

ifeq ($(HANDLE_MODEL), EVUV)
ifeq ($(PROTOCOL_STYLE), REDIS)
LIBA += -lsupex_evuv_redis
else
LIBA += -lsupex_evuv_http
endif
endif
