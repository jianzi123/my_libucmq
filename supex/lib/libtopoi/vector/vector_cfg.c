#include <json.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "vector_cfg.h"

static char *x_strdup(const char *src)
{ 
	if (src == NULL)
		return NULL;

	int len = strlen(src);
	char *out = calloc(len + 1, sizeof(char));
	strcpy(out, src);
	return out; 
}

void read_lrp_cfg(struct lrp_cfg_file *l_cfg, char *name)
{
        const char *str_val = NULL;
        struct json_object *tmp = NULL;
        struct json_object *obj = NULL;
        struct json_object *cfg = json_object_from_file( name );
        if (cfg == NULL) {
                goto fail;
        }
	/*poi*/
	if (json_object_object_get_ex(cfg, "poi_data", &tmp)) {
        } else goto fail;
	if (json_object_object_get_ex(tmp, "port", &obj)) {
                l_cfg->poi_port = (short)json_object_get_int(obj);
        } else goto fail;

	if (json_object_object_get_ex(tmp, "host", &obj)) {
                str_val = json_object_get_string(obj);
                l_cfg->poi_host = x_strdup(str_val);
        } else goto fail;
        
	if (json_object_object_get_ex(tmp, "username", &obj)) {
                str_val = json_object_get_string(obj);
                l_cfg->poi_username = x_strdup(str_val);
        } else goto fail;

	if (json_object_object_get_ex(tmp, "password", &obj)) {
                str_val = json_object_get_string(obj);
                l_cfg->poi_password = x_strdup(str_val);
        } else goto fail;

	if (json_object_object_get_ex(tmp, "database", &obj)) {
                str_val = json_object_get_string(obj);
                l_cfg->poi_database = x_strdup(str_val);
        } else goto fail;
	/*line*/
	return;
	fail:
	fprintf(stderr, "invalid lrp config file :%s\n", name);
	exit(0);
}
