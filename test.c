#include "libtg.h"
#include "mtx/include/api.h"
#include "mtx/include/buf.h"
#include "mtx/include/types.h"
#include "tl/deserialize.h"
#include "tl/id.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "tl/alloc.h"
#include "tl/names.h"

char * callback(
			void *userdata,
			TG_AUTH auth,
			tl_user_t *user)
{
	switch (auth) {
		case TG_AUTH_SENDCODE:
			{
				int code;
				printf("enter code: \n");
				scanf("%d", &code);
				printf("code: %d\n", code);
				char phone_code[32];
				sprintf(phone_code, "%d", code);
				return strdup(phone_code);
			}
			break;
		
		default:
			break;
	}

	return NULL;
}

int main(int argc, char *argv[])
{
	tg_t *tg = tg_new(
			"test.db", 
			24646404, 
			"818803c99651e8b777c54998e6ded6a0");
	
	char phone[32];
	printf("enter phone number (+7XXXXXXXXXX): \n");
	scanf("%s", phone);
	printf("phone number: %s\n", phone);

	tg_connect(tg, 
			phone, NULL, callback);

	return 0;
}
