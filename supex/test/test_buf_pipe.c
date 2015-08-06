#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>

#if 0

int main(void)
{
	int tube[2];
	char c = 'c';
	int i;

	fprintf(stdout, "Tube Creation\n");
	fprintf(stdout, "Theoretical max size: %d\n", PIPE_BUF);
	if( pipe(tube) != 0)
	{
		perror("pipe");
		_exit(1);
	}
	fprintf(stdout, "Writing in pipe\n");
	for(i=0;; i++)
	{
		fprintf(stdout, "%d bytes written\n", i+1);
		if( write(tube[1], &c, 1) != 1)
		{
			perror("Write");
			_exit(1);
		}
	}
	return 0;
}
#endif


#if 0
char g_buf[PIPE_BUF] = {};

int main(void)
{
	int tube[2];
	char c = 'c';
	int i;

	fprintf(stdout, "Tube Creation\n");
	fprintf(stdout, "Theoretical max size: %d\n", PIPE_BUF);
	if( pipe(tube) != 0)
	{
		perror("pipe");
		_exit(1);
	}
	fprintf(stdout, "Writing in pipe\n");
	for(i=0;; i++)
	{
		if (i == 10){
			if( write(tube[1], g_buf, 1) != 1)
			{
				perror("Write");
				_exit(1);
			}
			fprintf(stdout, "%d bytes written\n", 1);
		}
		if( write(tube[1], g_buf, PIPE_BUF) != PIPE_BUF)
		{
			perror("Write");
			_exit(1);
		}
		fprintf(stdout, "%d bytes written\n", (i+1)*PIPE_BUF);
	}
	return 0;
}

#endif


#if 0
char g_buf[PIPE_BUF+1] = {};

int main(void)
{
	int tube[2];
	char c = 'c';
	int i;

	fprintf(stdout, "Tube Creation\n");
	fprintf(stdout, "Theoretical max size: %d\n", PIPE_BUF);
	if( pipe2(tube, (O_NONBLOCK | O_CLOEXEC)) < 0 )
	//if( pipe(tube) != 0)
	{
		perror("pipe");
		_exit(1);
	}
	fprintf(stdout, "Writing in pipe\n");
	for(i=0;; i++)
	{
		if (i == 2){
			int size = write(tube[1], g_buf, PIPE_BUF + 1);
			fprintf(stdout, "%d bytes written\n", size);
		}
		int size = write(tube[1], g_buf, PIPE_BUF);
		if (size != PIPE_BUF)
		{
			perror("Write");
			printf("%d\n", size);
			_exit(1);
		}
		fprintf(stdout, "%d bytes written\n", (i+1)*PIPE_BUF);
	}
	return 0;
}

#endif


#if 0
char g_buf[PIPE_BUF] = {};

int main(void)
{
	int tube[2];
	char c = 'c';
	int i;

	fprintf(stdout, "Tube Creation\n");
	fprintf(stdout, "Theoretical max size: %d\n", PIPE_BUF);
	if( pipe(tube) != 0)
	{
		perror("pipe");
		_exit(1);
	}
	fcntl(tube[1], F_SETPIPE_SZ, (1<<4)*1024);// 4096开始每次每位<<1,最大1024*1024
	int size = 0;
	size =fcntl(tube[1], F_GETPIPE_SZ, NULL);
	printf("size is %d == %d\n", 1<<4, size/1024);
	fprintf(stdout, "Writing in pipe\n");
	for(i=0;; i++)
	{
		if( write(tube[1], g_buf, PIPE_BUF) != PIPE_BUF)
		{
			perror("Write");
			_exit(1);
		}
		fprintf(stdout, "%d bytes written\n", (i+1)*PIPE_BUF);
	}
	return 0;
}

#endif

#if 1
char g_buf[PIPE_BUF] = {};

int main(void)
{
	int tube[2];
	char c = 'c';
	int i;

	fprintf(stdout, "Tube Creation\n");
	fprintf(stdout, "Theoretical max size: %d\n", PIPE_BUF);
	if( pipe2(tube, (O_NONBLOCK | O_CLOEXEC)) < 0 )
	{
		perror("pipe");
		_exit(1);
	}
	int size = 0;
	fprintf(stdout, "Writing in pipe\n");
	size = write(tube[1], g_buf, 2);
	fprintf(stdout, "%d bytes written\n", size);
	size = read(tube[0], g_buf, 2);
	fprintf(stdout, "%d bytes read\n", size);
	size = read(tube[0], g_buf, 2);
	fprintf(stdout, "%d bytes read\n", size);

	size = write(tube[1], g_buf, 3);
	fprintf(stdout, "%d bytes written\n", size);
	size = read(tube[0], g_buf, 4);
	fprintf(stdout, "%d bytes read\n", size);
	size = read(tube[0], g_buf, 4);
	fprintf(stdout, "%d bytes read\n", size);
	return 0;
}

#endif


///see /proc/sys/fs/pipe-max-size
