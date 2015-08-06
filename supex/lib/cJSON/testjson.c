#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"

void main()
{
	char text[] = "{\"aa\":22,\"bb\":\"\"}";
	char *out;

	cJSON *root=cJSON_Parse(text);
	if (!root) {printf("Error before: [%s]\n",cJSON_GetErrorPtr());}
	else
	{
		out=cJSON_Print(root);
		printf("%s\n",out);
		free(out);
		if ( !cJSON_GetObjectItem(root, "cc") )
			printf("NULL\n");

		int aa = cJSON_GetObjectItem(root, "aa")->valueint;
		printf("%d\n", aa);
		char *bb = cJSON_GetObjectItem(root, "bb")->valuestring;
		printf("%s\n", bb);
		cJSON_Delete(root);
	}
}
