#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "conhash.h"
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <stdio.h>


struct conhash_node {
	struct node_s node;
	struct conhash_node *next;
};

struct conhash_ring {
	int count;
	struct conhash_s *root;
	struct conhash_node *head;
};


static int init(lua_State *L)
{
	/* init conhash instance */

	struct conhash_ring *ring = calloc(sizeof(struct conhash_ring), 1);
	if (NULL == ring) {
		goto INIT_FAIL;
	}
	ring->root = conhash_init(NULL);
	if(NULL == ring->root){
		perror("conhash_init error");
		goto INIT_FAIL;
	}

	lua_pushinteger(L,(long)ring);
	return 1;
INIT_FAIL:
	lua_pushnil(L);
	return 1;
}

static int destory(lua_State *L)
{
	/* init conhash instance */
	struct conhash_ring *ring = lua_tointeger(L, 1);

	if (NULL == ring || NULL == ring->root){
		return 0;
	}
	conhash_fini(ring->root);

	struct conhash_node *temp = ring->head;
	struct conhash_node *next = NULL;
	while (temp != NULL){
		next = temp->next;
		free(temp);
		temp = next;
	}
	free(ring);
	return 0;
}

static int set(lua_State *L)
{
	if(3 != lua_gettop(L)){
		lua_pushboolean(L, 0);
		return 1;
	}
	struct conhash_ring *ring = lua_tointeger(L, 1);
	if (NULL == ring || NULL == ring->root){
		lua_pushboolean(L, 0);
		return 1;
	}
	const char *node_name = lua_tostring (L, 2);
	int vtnodes = lua_tointeger(L, 3);

	/* set node */
	struct conhash_node *head = ring->head;
	struct conhash_node *node = calloc(sizeof(struct conhash_node), 1);
	assert(node);
	ring->head = node;
	node->next = head;
	ring->count ++;
	
	conhash_set_node(node, node_name, vtnodes);
        conhash_add_node(ring->root, node);
        //printf("virtual nodes number %d\n", conhash_get_vnodes_num(g_conhash));
	lua_pushboolean(L, 1);
	return 1;
}
#if 0
static int del(lua_State *L)
{
	if(!g_conhash){
		perror("g_conhash");
		return 1;
	}
	//const char *node_name = lua_tostring (L, 1);
	int id = lua_tointeger(L, 1);
	/* set node */
        //conhash_del_node(g_conhash, g_nodes[id]);
        printf("virtual nodes number %d\n", conhash_get_vnodes_num(g_conhash));
	return 0;
}
static int get(lua_State *L)
{
	if(!g_conhash){
		perror("g_conhash");
		return 1;
	}
	long hashes[512];
	/* set node */
	conhash_get_vnodes(g_conhash, hashes, sizeof(hashes)/sizeof(hashes[0]));
	//printf("virtual nodes number %d\n", conhash_get_vnodes_num(g_conhash));
	lua_pushstring(L, hashes);
	return 1;
}
#endif
static int lookup(lua_State *L)
{
	if(2 != lua_gettop(L)){
		lua_pushnil(L);
		return 1;
	}
	struct conhash_ring *ring = lua_tointeger(L, 1);
	if (NULL == ring || NULL == ring->root){
		lua_pushnil(L);
		return 1;
	}
	const char *object_name = lua_tostring (L, 2);

        /* try object */
	const struct node_s *p_node = conhash_lookup(ring->root, object_name);
	if (NULL == p_node){
		perror("conhash_lookup");
		lua_pushnil(L);
		return 1;
	}
	//printf("[%16s] is in node: [%s]\n", object_name, p_node->iden);
	lua_pushstring(L, p_node->iden);
	return 1;
}

static const struct luaL_Reg lib[] =
{
	{"init", init},
	{"destory", destory},
	{"set", set},
	//{"get", get},
	//{"del", del},
	{"lookup", lookup},
	{NULL, NULL}
};

int luaopen_conhash(lua_State *L) {
	luaL_register(L, "conhash", lib);
	return 1;
}
