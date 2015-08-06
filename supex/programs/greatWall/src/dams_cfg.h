#pragma once

#define MAX_LINK_INDEX		32
#define NO_SET_UP		'0'
#define IS_SET_UP		'1'

struct dams_link {
	short port;
	char *host;
};

struct dams_cfg_file {
	int count;						//links的个数
	char fresh[ MAX_LINK_INDEX ];	//刷新时间
	char delay[ MAX_LINK_INDEX ];	//延迟时间
	struct dams_link links[ MAX_LINK_INDEX ];
};

void read_dams_cfg(struct dams_cfg_file *p_cfg, char *name);
