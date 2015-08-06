#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "load_cfg.h"
#include "json.h"

void load_cfg_argv(struct smart_cfg_argv *p_cfg, int argc ,char** argv)
{
	strncat(p_cfg->msmq_name, strrchr(argv[0], 0x2f), MAX_FILE_NAME_SIZE - 1);
	strncat(p_cfg->serv_name, (strrchr(argv[0], 0x2f) + 1), MAX_FILE_NAME_SIZE - 1);
	char opt;
	short cfg = false;
	while((opt = getopt(argc,argv, "c:h:p"))!= -1) {
		switch(opt){
			case 'c':
				cfg = true;
				strncat(p_cfg->conf_name, optarg, MAX_FILE_NAME_SIZE - 1);
				printf("load config:'%s'\n", optarg);
				break;
			default:
				printf("unknow option :%c\n", opt);
				printf("\x1B[0;32mUSE LIKE :\n\x1B[1;31m\t./%s -c %s_conf.json\n\x1B[m", p_cfg->serv_name, p_cfg->serv_name);
				exit(EXIT_FAILURE);
		}
	}
	if(!cfg){
		int len = strlen(p_cfg->serv_name);
		strncat(p_cfg->conf_name, p_cfg->serv_name, MAX_FILE_NAME_SIZE - 1);
		strncat(p_cfg->conf_name, "_conf.json", MAX_FILE_NAME_SIZE - len -1);
	}
}

void load_cfg_file(struct smart_cfg_file *p_cfg, char *name)
{
        const char *str_val = NULL;
        struct json_object *obj = NULL;
        struct json_object *cfg = json_object_from_file( name );
        if (cfg == NULL) {
                goto fail;
        }

	if (json_object_object_get_ex(cfg, "service_port", &obj)) {
                p_cfg->srv_port = (short)json_object_get_int(obj);
        } else goto fail;
        
	if (json_object_object_get_ex(cfg, "hander_counts", &obj)) {
                p_cfg->hander_counts = (short)json_object_get_int(obj);
        } else goto fail;

        if (json_object_object_get_ex(cfg, "worker_counts", &obj)) {
                p_cfg->worker_counts = (short)json_object_get_int(obj);
        } else goto fail;

#ifdef OPEN_CORO
	if (json_object_object_get_ex(cfg, "tasker_counts", &obj)) {
                p_cfg->tasker_counts = (short)json_object_get_int(obj);
        } else goto fail;
#endif
	if (json_object_object_get_ex(cfg, "max_req_size", &obj)) {
                p_cfg->max_req_size = json_object_get_int(obj);
        } else goto fail;



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
	return;
fail:
	x_printf(D, "invalid config file :%s\n", name);
	exit(EXIT_FAILURE);
}
