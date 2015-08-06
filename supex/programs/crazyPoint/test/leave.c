/*
   struct ev_child child_watcher[ MAX_APP_COUNTS ];
*/
static void child_callback (struct ev_loop *loop, ev_child *w, int revents)
{
	ev_child_stop (loop, w); 
	x_printf ("process %d exited with status %x\n", w->rpid, w->rstatus); 
} 


int main(int argc ,char** argv)
{
	int i = 0;
	pid_t pid[ MAX_APP_COUNTS ] = {0};

	first_init();

	/*init fifo*/
	fifo_init(G_PAGE_SIZE);
	
	for(i=0; i < G_APP_COUNTS; i++){
		pid[i] = fork();
		if (pid[i] < 0){
			x_perror("fork");
			exit(EXIT_FAILURE);
		}else if (pid[i] == 0){
			G_APP_IDX = i;
			xxxxxx//FIXME
		}else{
			printf("FORK ONE PROCESS -->PID :%d\n", pid[i]);
		}
	}

	/*==start master==*/
	/*init socket*/
	int listenfd = socket_init( g_cfg_pool.srv_port );
	/*set loop*/
	g_master_thread.thread_id = pthread_self();
	g_master_thread.loop = ev_default_loop (0);
	
	/*listern child*/
	for(i=0; i < G_APP_COUNTS; i++){
		ev_child_init (&g_master_thread.child_watcher[i], child_callback, pid[i], 0);
		ev_child_start (g_master_thread.loop, &g_master_thread.child_watcher[i]);
	}

	/*register signal*/
	ev_signal_init( &(g_master_thread.signal_watcher), signal_callback, SIGQUIT );
	ev_signal_start( g_master_thread.loop, &(g_master_thread.signal_watcher) );

	/*set update timer*/
	ev_timer_init( &(g_master_thread.update_watcher), update_callback, get_overplus_time(), 0. );
	ev_timer_start( g_master_thread.loop, &(g_master_thread.update_watcher) );

	/*set io_watcher*/
	ev_io_init( &(g_master_thread.accept_watcher), accept_callback, listenfd, EV_READ);
	ev_io_start( g_master_thread.loop, &(g_master_thread.accept_watcher) );

				//char *res = put_fifo_msg("add task\n", 9);
	/*start loop*/
	ev_loop(g_master_thread.loop, 0);
	/*exit*/
	ev_loop_destroy(g_master_thread.loop);
	printf("master fun exit!\n");
	return 0;
}
