#pragma once

#include "utils.h"

struct net_cache {
	int max_size;
	int get_size;
	int out_size;
	char *buf_addr;
	char base[MAX_DEF_LEN];
};

void cache_peak(unsigned int max);

void cache_init(struct net_cache *info);

int cache_add(struct net_cache *info, const char *data, int size);

int cache_set(struct net_cache *info, const char *data, int max, int size);

ssize_t net_recv(struct net_cache *cache, int sockfd, int *status);

ssize_t net_send(struct net_cache *cache, int sockfd, int *status);

void cache_free(struct net_cache *info);
