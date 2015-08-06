#pragma once 

struct lrp_cfg_file {
	short poi_port;
	char * poi_host;
	char * poi_username;
	char * poi_password;
	char * poi_database;
};

void read_lrp_cfg(struct lrp_cfg_file * l_cfg,char * name);


