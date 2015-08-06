#include "utils.h"

#include "share_evcb.h"

void msmq_share_cb(struct ev_loop *loop, ev_io *w, int revents)
{
	int n = msmq_call();
	x_printf(D, "done shell cntl %d\n", n);
	return;
}
