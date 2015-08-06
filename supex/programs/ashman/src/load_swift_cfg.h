#pragma once

#include "swift_cfg.h"


void load_swift_cfg_argv(struct swift_cfg_argv *p_cfg, int argc ,char** argv);

void load_swift_cfg_file(struct swift_cfg_file *p_cfg, char *name);
