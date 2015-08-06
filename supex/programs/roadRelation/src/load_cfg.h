#pragma once

#include "smart_cfg.h"


void load_cfg_argv(struct smart_cfg_argv *p_cfg, int argc ,char** argv);

void load_cfg_file(struct smart_cfg_file *p_cfg, char *name);
