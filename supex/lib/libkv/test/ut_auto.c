#include "ut_public.h"






/** version */
void case_version();
/** init */
void case_init();
void case_init_multiple();
void case_init_without_used();
void case_init_after_used();
void case_used_without_init();
void case_init_stress();
/** set */
void case_cmd_set_normal();
void case_cmd_set_invalid_param();
void case_cmd_set_invalid_type();
void case_cmd_set_stress();
/** get */
void case_cmd_get();
/** del */
void case_cmd_del();
/** dbsize */
void case_cmd_dbsize();
/** flushdb */
void case_cmd_flushdb();
/** incr */
void case_cmd_incr();
/** incrby */
void case_cmd_incrby();
/** decr */
void case_cmd_decr();
/** decrby */
void case_cmd_decrby();
/** used memory */
void case_used_memory();
/** lpush */
void case_cmd_lpush_add_few_bytes();
void case_cmd_lpush_add_lot_bytes();
void case_cmd_lpush_del();
void case_cmd_lpush_invalid_param();
/** exists */
void case_cmd_exists_normal();
void case_cmd_exists_invalid_cmd();
void case_cmd_exists_invalid_param();
/** lrange */
void case_cmd_lrange_without_found();
void case_cmd_lrange_found();
void case_cmd_lrange_uninit();
void case_cmd_lrange_invalid_param();
void case_cmd_lrange_mem_leak();
/** sadd */
void cmd_case_sadd_normal();
void cmd_case_sadd_invalid_cmd();
void cmd_case_sadd_invalid_param();
void cmd_case_sadd_invalid_type();
void cmd_case_sadd_stress();
/** smembers */
void cmd_case_smembers_without_members();
void cmd_case_smembers_with_members();
void cmd_case_smembers_invalid_cmd();
void cmd_case_smembers_invalid_param();
void cmd_case_smembers_invalid_type();
void cmd_case_smembers_stress();
/** expire */
void case_cmd_expire_get();
void case_cmd_expire_list();
void case_cmd_expire_set();
void case_cmd_expire_without_key();
void case_cmd_expire_invalid_cmd();
void case_cmd_expire_invalid_param();
void case_cmd_expire_stress();
/** expireat */
void case_cmd_expireat_get();
void case_cmd_expireat_list();
void case_cmd_expireat_set();
void case_cmd_expireat_without_key();
void case_cmd_expireat_invalid_cmd();
void case_cmd_expireat_invalid_param();
void case_cmd_expireat_stress();
/** pexpire */
void case_cmd_pexpire_get();
void case_cmd_pexpire_list();
void case_cmd_pexpire_set();
void case_cmd_pexpire_without_key();
void case_cmd_pexpire_invalid_cmd();
void case_cmd_pexpire_invalid_param();
void case_cmd_pexpire_stress();
/** pexpireat */
void case_cmd_pexpireat_get();
void case_cmd_pexpireat_list();
void case_cmd_pexpireat_set();
void case_cmd_pexpireat_without_key();
void case_cmd_pexpireat_invalid_cmd();
void case_cmd_pexpireat_invalid_param();
void case_cmd_pexpireat_stress();






#ifdef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();

        /** version */
        UTIL_ADD_CASE_SHORT(case_version);
        /** init */
        UTIL_ADD_CASE_SHORT(case_init);
        UTIL_ADD_CASE_SHORT(case_init_multiple);
        UTIL_ADD_CASE_SHORT(case_init_without_used);
        UTIL_ADD_CASE_SHORT(case_init_after_used);
        UTIL_ADD_CASE_SHORT(case_used_without_init);
        UTIL_ADD_CASE_SHORT(case_init_stress);
        /** set */
        UTIL_ADD_CASE_SHORT(case_cmd_set_normal);
        UTIL_ADD_CASE_SHORT(case_cmd_set_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_set_invalid_type);
        UTIL_ADD_CASE_SHORT(case_cmd_set_stress);
        /** get */
        UTIL_ADD_CASE_SHORT(case_cmd_get);
        /** del */
        UTIL_ADD_CASE_SHORT(case_cmd_del);
        /** dbsize */
        UTIL_ADD_CASE_SHORT(case_cmd_dbsize);
        /** flushdb */
        UTIL_ADD_CASE_SHORT(case_cmd_flushdb);
        /** incr */
        UTIL_ADD_CASE_SHORT(case_cmd_incr);
        /** incrby */
        UTIL_ADD_CASE_SHORT(case_cmd_incrby);
        /** decr */
        UTIL_ADD_CASE_SHORT(case_cmd_decr);
        /** decrby */
        UTIL_ADD_CASE_SHORT(case_cmd_decrby);
        /** used memory */
        UTIL_ADD_CASE_SHORT(case_used_memory);
        /** lpush */
        UTIL_ADD_CASE_SHORT(case_cmd_lpush_add_few_bytes);
        UTIL_ADD_CASE_SHORT(case_cmd_lpush_add_lot_bytes);
        UTIL_ADD_CASE_SHORT(case_cmd_lpush_del);
        UTIL_ADD_CASE_SHORT(case_cmd_lpush_invalid_param);
        /** exists */
        UTIL_ADD_CASE_SHORT(case_cmd_exists_normal);
        UTIL_ADD_CASE_SHORT(case_cmd_exists_invalid_cmd);
        UTIL_ADD_CASE_SHORT(case_cmd_exists_invalid_param);
        /** lrange */
        UTIL_ADD_CASE_SHORT(case_cmd_lrange_without_found);
        UTIL_ADD_CASE_SHORT(case_cmd_lrange_found);
        UTIL_ADD_CASE_SHORT(case_cmd_lrange_uninit);
        UTIL_ADD_CASE_SHORT(case_cmd_lrange_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_lrange_mem_leak);
        /** sadd */
        UTIL_ADD_CASE_SHORT(cmd_case_sadd_normal);
        UTIL_ADD_CASE_SHORT(cmd_case_sadd_invalid_cmd);
        UTIL_ADD_CASE_SHORT(cmd_case_sadd_invalid_param);
        UTIL_ADD_CASE_SHORT(cmd_case_sadd_invalid_type);
        UTIL_ADD_CASE_SHORT(cmd_case_sadd_stress);
        /** smembers */
        UTIL_ADD_CASE_SHORT(cmd_case_smembers_without_members);
        UTIL_ADD_CASE_SHORT(cmd_case_smembers_with_members);
        UTIL_ADD_CASE_SHORT(cmd_case_smembers_invalid_cmd);
        UTIL_ADD_CASE_SHORT(cmd_case_smembers_invalid_param);
        UTIL_ADD_CASE_SHORT(cmd_case_smembers_invalid_type);
        UTIL_ADD_CASE_SHORT(cmd_case_smembers_stress);
        /** expire */
        UTIL_ADD_CASE_SHORT(case_cmd_expire_get);
        UTIL_ADD_CASE_SHORT(case_cmd_expire_list);
        UTIL_ADD_CASE_SHORT(case_cmd_expire_set);
        UTIL_ADD_CASE_SHORT(case_cmd_expire_without_key);
        UTIL_ADD_CASE_SHORT(case_cmd_expire_invalid_cmd);
        UTIL_ADD_CASE_SHORT(case_cmd_expire_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_expire_stress);
        /** expireat */
        UTIL_ADD_CASE_SHORT(case_cmd_expireat_get);
        UTIL_ADD_CASE_SHORT(case_cmd_expireat_list);
        UTIL_ADD_CASE_SHORT(case_cmd_expireat_set);
        UTIL_ADD_CASE_SHORT(case_cmd_expireat_without_key);
        UTIL_ADD_CASE_SHORT(case_cmd_expireat_invalid_cmd);
        UTIL_ADD_CASE_SHORT(case_cmd_expireat_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_expireat_stress);
        /** pexpire */
        UTIL_ADD_CASE_SHORT(case_cmd_pexpire_get);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpire_list);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpire_set);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpire_without_key);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpire_invalid_cmd);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpire_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpire_stress);
        /** pexpireat */
        UTIL_ADD_CASE_SHORT(case_cmd_pexpireat_get);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpireat_list);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpireat_set);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpireat_without_key);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpireat_invalid_cmd);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpireat_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpireat_stress);
        
        UTIL_RUN();
        UTIL_UNINIT();
  
        return 0;
}
#endif
