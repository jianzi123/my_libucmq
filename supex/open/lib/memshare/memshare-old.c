#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//#define _GNU_SOURCE 
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <semaphore.h>
#include <unistd.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define FILE_MODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define PAGE_SIZE getpagesize(  )
//#define MAX_MEM_DATA_SIZE (PAGE_SIZE * PAGE_SIZE)
#define MAX_NAME_SIZE 32



struct shmhead
{
	int data_size;
	int data_used;
	char shmname[MAX_NAME_SIZE];
	char semname[MAX_NAME_SIZE];
};
/*<--------------------------------API------------------------------->*/
sem_t *_sem_create(const char *name)
{
	sem_t  *mutex;
	if((mutex = sem_open(name, O_CREAT|O_EXCL,FILE_MODE, 1)) == SEM_FAILED)
	{
		printf("sem_open error");
		exit(-1);
	}
	return mutex;
}
sem_t *_sem_open(const char *name)
{
	sem_t  *mutex;
	if((mutex = sem_open(name, 0)) == SEM_FAILED)
	{
		printf("sem_open error");
		exit(-1);
	}
	return mutex;
}

void _sem_lock(sem_t  *mutex)
{
	sem_wait(mutex);
}

void _sem_unlock(sem_t  *mutex)
{
	sem_post(mutex);
}

void _sem_close(sem_t  *mutex)
{
	sem_close(mutex);
}

void _sem_delete(const char *name)
{
	sem_unlink(name);
}
/*<===>*/
static int sem_lock(lua_State *L)
{
	sem_t  *mutex;
	const char *semname = lua_tostring (L, 1);
	//open semaphore
	mutex = _sem_open(semname);
	//lock semaphore
	_sem_lock(mutex);
	//close semaphore
	_sem_close(mutex);
	return 0;
}
static int sem_unlock(lua_State *L)
{
	sem_t  *mutex;
	const char *semname = lua_tostring (L, 1);
	//open semaphore
	mutex = _sem_open(semname);
	//unlock semaphore
	_sem_unlock(mutex);
	//close semaphore
	_sem_close(mutex);
	return 0;
}

static int sem_new(lua_State *L)
{
	sem_t  *mutex;
	const char *semname = lua_tostring (L, 1);
	//delete semaphore
	_sem_delete(semname);
	//create semaphore
	mutex = _sem_create(semname);
	//close semaphore
	_sem_close(mutex);
	return 0;
}
static int sem_del(lua_State *L)
{
	const char *semname = lua_tostring (L, 1);
	//delete semaphore
	_sem_delete(semname);
	return 0;
}

/*==============================================================*/
//static int get(lua_State *L)
//static int set(lua_State *L)
static int pop(lua_State *L)
{
	int     memfd;
	struct shmhead *ptr;
	const char *shmname = NULL;
	char *semname = NULL;
	int shmsize = 0;
	int len = 0;
	char *data = NULL;
	//get args
	if(lua_gettop(L) == 1){
		shmname = lua_tostring (L, 1);
	}else{
		return 0;
	}

	//open share memory
	if((memfd = shm_open(shmname,O_RDWR,FILE_MODE)) == -1)
	{
		perror("shm_open error");
		exit(-1);
	}
	//memory map
	if((ptr = mmap(NULL, sizeof(struct shmhead), PROT_READ | PROT_WRITE,MAP_SHARED,memfd,0)) == MAP_FAILED)
	{
		perror("mmap error");
		exit(-1);
	}
	shmsize = ptr->data_size + sizeof(struct shmhead);
	//memory remap
	ptr = mremap(ptr, sizeof(struct shmhead), shmsize, MREMAP_MAYMOVE);
	data = (char *)ptr + sizeof(struct shmhead);
	semname = ptr->semname;
	close(memfd);

	/*==============================================================*/
	sem_t  *mutex;
	//open semaphore
	mutex = _sem_open(semname);
	//lock semaphore
	_sem_lock(mutex);
	//pop method
	lua_pushlstring(L, (const char *)data, ptr->data_used);
	ptr->data_used = 0;
	memset(data, 0, (size_t)ptr->data_size);
	//memory unmap
	munmap(ptr, shmsize);
	//unlock semaphore
	_sem_unlock(mutex);
	//close semaphore
	_sem_close(mutex);
	return 1;
}
static int push(lua_State *L)
{
	int     memfd;
	struct shmhead *ptr;
	const char *shmname = NULL;
	char *semname = NULL;
	const char *pushdata = NULL;
	int shmsize = 0;
	int len = 0;
	char *data = NULL;
	//get args
	if(lua_gettop(L) == 2){
		shmname = lua_tostring (L, 1);
		pushdata = lua_tostring (L, 2);
	}else{
		return 0;
	}

	//open share memory
	if((memfd = shm_open(shmname,O_RDWR,FILE_MODE)) == -1)
	{
		perror("shm_open error");
		exit(-1);
	}
	//memory map
	if((ptr = mmap(NULL, sizeof(struct shmhead), PROT_READ | PROT_WRITE,MAP_SHARED,memfd,0)) == MAP_FAILED)
	{
		perror("mmap error");
		exit(-1);
	}
	shmsize = ptr->data_size + sizeof(struct shmhead);
	//memory remap
	ptr = mremap(ptr, sizeof(struct shmhead), shmsize, MREMAP_MAYMOVE);
	data = (char *)ptr + sizeof(struct shmhead);
	semname = ptr->semname;
	close(memfd);

	/*==============================================================*/
	sem_t  *mutex;
	//open semaphore
	mutex = _sem_open(semname);
	//lock semaphore
	_sem_lock(mutex);
	//push method
	int svlen = strlen(pushdata);
	int lvlen = ptr->data_size - ptr->data_used;
	svlen = (lvlen > svlen ) ? svlen : lvlen;
	if(svlen > 0){
		memcpy(data + ptr->data_used, pushdata, svlen);
		ptr->data_used += svlen;
	}
	//memory unmap
	munmap(ptr, shmsize);
	//unlock semaphore
	_sem_unlock(mutex);
	//close semaphore
	_sem_close(mutex);
	return 0;
}

static int new(lua_State *L)
{
	int     memfd;
	struct shmhead *ptr;
	const char *shmname = NULL;
	const char *semname = NULL;
	int shmsize = 0; 
	//get args
	if(lua_gettop(L) == 3){
		shmname = lua_tostring (L, 1);
		semname = lua_tostring (L, 2);
		shmsize = lua_tointeger (L, 3) * PAGE_SIZE; 
	}else{
		return 0;
	}
	//delete share memory
	shm_unlink(shmname);
	//create share memory
	if((memfd = shm_open(shmname,O_RDWR | O_CREAT | O_EXCL,FILE_MODE)) == -1)
	{
		perror("shm_open error");
		exit(-1);
	}
	//set share memory size
	ftruncate(memfd, shmsize);
	//memory map
	if((ptr = mmap(NULL, sizeof(struct shmhead), PROT_READ | PROT_WRITE,MAP_SHARED,memfd,0)) == MAP_FAILED)
	{
		perror("mmap error");
		exit(-1);
	}
	//init share memory
	ptr->data_size = shmsize - sizeof(struct shmhead);
	ptr->data_used = 0;
	memcpy(ptr->shmname, shmname, strlen(shmname));
	memcpy(ptr->semname, semname, strlen(semname));
	//memory unmap
	munmap(ptr, sizeof(struct shmhead));
	//close memfd
	close(memfd);
	/*==============================================================*/
	sem_t  *mutex;
	//delete semaphore
	_sem_delete(semname);
	//create semaphore
	mutex = _sem_create(semname);
	//close semaphore
	_sem_close(mutex);
	return 0;
}

static int del(lua_State *L)
{
	const char *shmname = NULL;
	const char *semname = NULL;
	if(lua_gettop(L) == 2){
		shmname = lua_tostring (L, 1);
		semname = lua_tostring (L, 2);
	}else{
		return 0;
	}
	//delete share memory
	shm_unlink(shmname);
	//delete semaphore
	_sem_delete(semname);
	return 0;
}

static const struct luaL_Reg lib[] =
{
	{"new", new},
	{"del", del},
	{"push", push},
	{"pop", pop},
	{NULL, NULL}
};

int luaopen_memshare(lua_State *L) {
	luaL_register(L, "memshare", lib);
	return 1;
}
