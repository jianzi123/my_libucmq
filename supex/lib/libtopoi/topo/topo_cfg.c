#include <json.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "topo_cfg.h"

static char *x_strdup(const char *src)
{ 
	if (src == NULL)
		return NULL;

	int len = strlen(src);
	char *out = calloc(len + 1, sizeof(char));
	strcpy(out, src);
	return out; 
}

void read_topo_cfg(struct topo_cfg_file *p_cfg, char *name)
{
        const char *str_val = NULL;
        struct json_object *tmp = NULL;
        struct json_object *obj = NULL;
        struct json_object *cfg = json_object_from_file( name );
        if (cfg == NULL) {
                goto fail;
        }
	/*node*/
	if (json_object_object_get_ex(cfg, "node_data", &tmp)) {
        } else goto fail;
	if (json_object_object_get_ex(tmp, "port", &obj)) {
                p_cfg->node_port = (short)json_object_get_int(obj);
        } else goto fail;

	if (json_object_object_get_ex(tmp, "host", &obj)) {
                str_val = json_object_get_string(obj);
                p_cfg->node_host = x_strdup(str_val);
        } else goto fail;
        
	if (json_object_object_get_ex(tmp, "username", &obj)) {
                str_val = json_object_get_string(obj);
                p_cfg->node_username = x_strdup(str_val);
        } else goto fail;

	if (json_object_object_get_ex(tmp, "password", &obj)) {
                str_val = json_object_get_string(obj);
                p_cfg->node_password = x_strdup(str_val);
        } else goto fail;

	if (json_object_object_get_ex(tmp, "database", &obj)) {
                str_val = json_object_get_string(obj);
                p_cfg->node_database = x_strdup(str_val);
        } else goto fail;
	/*line*/
	if (json_object_object_get_ex(cfg, "line_data", &tmp)) {
        } else goto fail;
	if (json_object_object_get_ex(tmp, "port", &obj)) {
                p_cfg->line_port = (short)json_object_get_int(obj);
        } else goto fail;

	if (json_object_object_get_ex(tmp, "host", &obj)) {
                str_val = json_object_get_string(obj);
                p_cfg->line_host = x_strdup(str_val);
        } else goto fail;
        
	if (json_object_object_get_ex(tmp, "username", &obj)) {
                str_val = json_object_get_string(obj);
                p_cfg->line_username = x_strdup(str_val);
        } else goto fail;

	if (json_object_object_get_ex(tmp, "password", &obj)) {
                str_val = json_object_get_string(obj);
                p_cfg->line_password = x_strdup(str_val);
        } else goto fail;

	if (json_object_object_get_ex(tmp, "database", &obj)) {
                str_val = json_object_get_string(obj);
                p_cfg->line_database = x_strdup(str_val);
        } else goto fail;

	return;
fail:
	fprintf(stderr, "invalid topo config file :%s\n", name);
	exit(0);
}
