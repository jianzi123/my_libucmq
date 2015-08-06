#include <string.h>
#include <assert.h>

#include "json.h"
#include "load_crzpt_cfg.h"

void load_crzpt_cfg_file(struct crzpt_cfg_temp_data *p_cfg, char *name)
{
        const char *str_val = NULL;
        struct json_object *obj = NULL;
        struct json_object *cfg = json_object_from_file( name );
        if (cfg == NULL) {
                goto fail;
        }

	if (json_object_object_get_ex(cfg, "roamer_counts", &obj)) {
                p_cfg->roamer_counts = (short)json_object_get_int(obj);
        } else goto fail;
#ifdef OPEN_CORO
	if (json_object_object_get_ex(cfg, "pauper_counts", &obj)) {
                p_cfg->pauper_counts = (short)json_object_get_int(obj);
        } else goto fail;
#endif

	if (json_object_object_get_ex(cfg, "crzpt_online", &obj)) {
		p_cfg->app_counts = MIN( json_object_array_length(obj), MAX_APP_COUNTS );

		int i = 0;
		struct json_object* itr_obj = NULL;
		memset( p_cfg->app_names, 0, MAX_APP_COUNTS *(MAX_FILE_NAME_SIZE) );
		memset( p_cfg->app_msmqs, 0, MAX_APP_COUNTS *(MAX_FILE_NAME_SIZE) );
		for(i=0; i < p_cfg->app_counts; i++){
			itr_obj = json_object_array_get_idx(obj, i);
			str_val = json_object_get_string( itr_obj );
			strncpy( &p_cfg->app_names[i][0], str_val, MIN(strlen(str_val), MAX_FILE_NAME_SIZE -1) );
			
			p_cfg->app_msmqs[i][0] = '/';
			strncpy( &p_cfg->app_msmqs[i][1], str_val, MIN(strlen(str_val), MAX_FILE_NAME_SIZE -2) );
		}
        } else goto fail;

	return;
fail:
	x_printf(D, "invalid config file :%s\n", name);
	exit(EXIT_FAILURE);
}
