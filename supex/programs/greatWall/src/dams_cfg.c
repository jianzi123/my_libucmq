#include <json.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "dams_cfg.h"

static char *x_strdup(const char *src)
{ 
	if (src == NULL)
		return NULL;

	int len = strlen(src);
	char *out = calloc(len + 1, sizeof(char));
	assert(out);
	strcpy(out, src);
	return out; 
}

/*
 *函 数:read_dams_cfg
 *功 能:读取配置文件
 */
void read_dams_cfg(struct dams_cfg_file *p_cfg, char *name)
{
	int i = 0;
	int idx = 0;
	int add = 0;
        const char *str_val = NULL;
        struct json_object *tmp = NULL;
        struct json_object *ary = NULL;
        struct json_object *obj = NULL;
        struct json_object *cfg = json_object_from_file( name );
        if (cfg == NULL) {
                goto fail;
        }
	/*links*/
	if (json_object_object_get_ex(cfg, "links", &ary)) {
		add = json_object_array_length(ary);
		p_cfg->count = add;
		assert( p_cfg->count <= MAX_LINK_INDEX );

		for(i=0; i < add; i++){
			tmp = json_object_array_get_idx(ary, i);
			if (json_object_object_get_ex(tmp, "port", &obj)) {
				p_cfg->links[i].port = (short)json_object_get_int(obj);
			} else goto fail;
			if (json_object_object_get_ex(tmp, "host", &obj)) {
				str_val = json_object_get_string(obj);
				p_cfg->links[i].host = x_strdup(str_val);
			} else goto fail;
		}
        } else goto fail;
	/*fresh*/
	memset(p_cfg->fresh, NO_SET_UP, MAX_LINK_INDEX);
	if (json_object_object_get_ex(cfg, "fresh", &ary)) {
		add = json_object_array_length(ary);
		assert( (add <= p_cfg->count) && (add <= MAX_LINK_INDEX) );

		for(i=0; i < add; i++){
			obj = json_object_array_get_idx(ary, i);
			idx = (short)json_object_get_int(obj);
			assert( (idx <= p_cfg->count) && (idx <= MAX_LINK_INDEX) );
			p_cfg->fresh[idx] = IS_SET_UP;
		}
        } else goto fail;:q
	/*delay*/
	memset(p_cfg->delay, NO_SET_UP, MAX_LINK_INDEX);
	if (json_object_object_get_ex(cfg, "delay", &ary)) {
		add = json_object_array_length(ary);
		assert( (add <= p_cfg->count) && (add <= MAX_LINK_INDEX) );

		for(i=0; i < add; i++){
			obj = json_object_array_get_idx(ary, i);
			idx = (short)json_object_get_int(obj);
			assert( (idx <= p_cfg->count) && (idx <= MAX_LINK_INDEX) );
			p_cfg->delay[idx] = IS_SET_UP;
		}
        } else goto fail;
	

	return;
fail:
	fprintf(stderr, "invalid dams config file :%s\n", name);
	exit(0);
}
