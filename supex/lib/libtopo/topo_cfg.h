#pragma once

struct topo_cfg_file {
	short node_port;
	char *node_host;
	char *node_username;
	char *node_password;
	char *node_database;

	short line_port;
	char *line_host;
	char *line_username;
	char *line_password;
	char *line_database;
};

void read_topo_cfg(struct topo_cfg_file *p_cfg, char *name);
