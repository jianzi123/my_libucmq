#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "load_sniff_cfg.h"
#include "json.h"


/*
 *函 数:load_sniff_cfg_file
 *功 能:
 */
void load_sniff_cfg_file(struct sniff_cfg_file *p_cfg, char *name)
{
        const char *str_val = NULL;
        struct json_object *obj = NULL;
        struct json_object *cfg = json_object_from_file( name );
        if (cfg == NULL) {
                goto fail;
        }

	if (json_object_object_get_ex(cfg, "parter_counts", &obj)) {
                p_cfg->parter_counts = (short)json_object_get_int(obj);
        } else goto fail;

#ifdef OPEN_CORO
	if (json_object_object_get_ex(cfg, "sharer_counts", &obj)) {
                p_cfg->sharer_counts = (short)json_object_get_int(obj);
        } else goto fail;
#endif

	if (json_object_object_get_ex(cfg, "log_path", &obj)) {
                str_val = json_object_get_string(obj);
                p_cfg->log_path = x_strdup(str_val);
        } else goto fail;
        
	if (json_object_object_get_ex(cfg, "log_file", &obj)) {
                str_val = json_object_get_string(obj);
                p_cfg->log_file = x_strdup(str_val);
        } else goto fail;

        if (json_object_object_get_ex(cfg, "log_level", &obj)) {
                p_cfg->log_level = json_object_get_int(obj);
        } else goto fail;
#if 0
#ifdef USE_HTTP_PROTOCOL
	if (json_object_object_get_ex(cfg, "api_apply", &obj)) {
		p_cfg->api_counts ++;
                str_val = json_object_get_string(obj);
                p_cfg->api_apply = x_strdup(str_val);
        }

	if (json_object_object_get_ex(cfg, "api_fetch", &obj)) {
		p_cfg->api_counts ++;
                str_val = json_object_get_string(obj);
                p_cfg->api_fetch = x_strdup(str_val);
        }
        
        if (json_object_object_get_ex(cfg, "api_merge", &obj)) {
		p_cfg->api_counts ++;
                str_val = json_object_get_string(obj);
                p_cfg->api_merge = x_strdup(str_val);
        }

	if (json_object_object_get_ex(cfg, "api_custom", &obj)) {
		int add = json_object_array_length(obj);
		p_cfg->api_counts += add;
		assert( p_cfg->api_counts <= MAX_API_COUNTS );

		int i = 0;
		struct json_object* itr_obj = NULL;
		memset( p_cfg->api_names, 0, MAX_API_COUNTS*(MAX_API_NAME_LEN + 1) );
		for(i=0; i < add; i++){
			itr_obj = json_object_array_get_idx(obj, i);
			str_val = json_object_get_string( itr_obj );
			assert( strlen(str_val) <= MAX_API_NAME_LEN );
			strncpy( &p_cfg->api_names[i][0], str_val, MIN(strlen(str_val), MAX_API_NAME_LEN) );
		}
	}
#endif
#endif
	return;
fail:
	x_printf(D, "invalid config file :%s\n", name);
	exit(EXIT_FAILURE);
}
