#pragma once

#define MAX_LINK_INDEX		32

struct weibo_link {
	short port;
	char *host;
};

struct weibo_cfg_file {
	
	int weibo_count;
	int static_count;
	int weidb_count;
	
	struct weibo_link weibo_store[ MAX_LINK_INDEX ];
	struct weibo_link weibo_static[ MAX_LINK_INDEX ];
	struct weibo_link weidb[ MAX_LINK_INDEX ];
};

void read_weibo_cfg(struct weibo_cfg_file *p_cfg, char *name);
