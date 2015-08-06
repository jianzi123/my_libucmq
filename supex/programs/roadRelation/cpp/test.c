#include <stdio.h>
#include <entry.h>
#include <interface.h>

int main(int argc, char * argv[])
{
	entry_init();
	fprintf(stdout, "[hello lua]\n");
	int max = 100;
	uint64_t slot[ max ];
	struct query_args info = {};
	info.idx = 10000115;
	info.buf = slot;
	info.peak = max;
	get_export_road_of_node(&info);
	int i;
	for (i = 0; i< info.size; i++) {
		fprintf(stdout, "[i:%d]\n", info.buf[i]);
	}

	return 0;
}
