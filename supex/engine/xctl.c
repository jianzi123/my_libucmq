#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>

#include <string.h>
#include <errno.h>
#include "utils.h"

#define MSMQ_NAME "/driview"
//#define MSMQ_NAME "/appcenter"
#define PRIOLOW 1
#define PRIOHIGH 2


int main(int argc, char **argv)
{
	/*get opt*/
	char ret;
	char opt;
	char mode;
	char *data = NULL;
	while((ret = getopt(argc,argv, "l:f:o:c:d:1234"))!= -1) {
		switch(ret){
			case 'l':
				opt = ret;
				data = optarg;
				printf("load mode:'%s'\n", optarg);
				break;
			case 'f':
				opt = ret;
				data = optarg;
				printf("free mode:'%s'\n", optarg);
				break;
			case 'o':
				opt = ret;
				data = optarg;
				printf("open mode:'%s'\n", optarg);
				break;
			case 'c':
				opt = ret;
				data = optarg;
				printf("close mode:'%s'\n", optarg);
				break;
			case 'd':
				opt = ret;
				data = optarg;
				printf("delete mode:'%s'\n", optarg);
				break;
			case '1':
			case '2':
			case '3':
			case '4':
				mode = ret;
				printf("method :'%c'\n", mode);
				break;
			default:
				printf("unknow option :%c\n", ret);
				exit(EXIT_FAILURE);
		}
	}
	if (argc != 4){
		printf("USE LIKE :./load -l api1 -[1,2,3,4]\n");
		exit(EXIT_FAILURE);
	}
	printf("data is %s\n",data);
	/*set opt*/
	struct msg_info cmd;
	memset(&cmd, 0, sizeof(struct msg_info));
	cmd.opt = opt;
	cmd.mode = mode;
	strcat(cmd.data, data);

	/*put opt*/
	mqd_t mqd = mq_open(MSMQ_NAME, O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, NULL);
	if (mqd == (mqd_t)-1) {
		perror("mq_open");
		exit(EXIT_FAILURE);
	}
	mq_send(mqd, (char *)&cmd, sizeof(struct msg_info), PRIOLOW);
	mq_close(mqd);
	return 0;
}
