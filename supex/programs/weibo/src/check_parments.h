#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <uuid/uuid.h>
#include <string.h>
#include <uuid/uuid.h>
#include <ctype.h>
#include "cJSON.h"
#include "weibo.h"
#define NGX_UNESCAPE_URI 1 
#define NGX_UNESCAPE_REDIRECT  2
#define NGX_UNESCAPE_URI_COMPONENT  0

#define WEIBO_SUCESS 0
#define WEIBO_FAILED -1
int upper_to_lower(char *p);

void  create_uuid(char *L);

void compare_time(long p);

int check_user(char *p);

int content_decode(u_char *dst, u_char *src);

