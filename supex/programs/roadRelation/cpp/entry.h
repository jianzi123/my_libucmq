#pragma once

#include <stdio.h>
#include <stdint.h>

#include "smart_api.h"

void entry_init(void);


int entry_cmd_erbr(struct data_node *p_node, uint64_t idx);

int entry_cmd_irbr(struct data_node *p_node, uint64_t idx);
int entry_cmd_rlbr(struct data_node *p_node, uint64_t idx);

