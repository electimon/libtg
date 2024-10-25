#include "libtg.h"
#include "mtx/include/buf.h"
#include "tg/tl_object.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{

	char phone[32];
	printf("enter phone number (+7XXXXXXXXXX): \n");
	scanf("%s", phone);
	printf("phone number: %s\n", phone);

	tg_t *tg = tg_new();

	tlo_t *codeSettings = tl_codeSettings(
			false,
		 	false,
		 	false,
		 	false,
		 	false, 
			false,
		 	NULL,
		 	0,
		 	NULL,
		 	NULL);

	tlo_t *sendCode = 
		tl_auth_sendCode(
				phone, 
				24646404, 
				"818803c99651e8b777c54998e6ded6a0", 
				codeSettings);

	tlo_t *initConnection = 
		tl_initConnection(
				24646404, 
				"telegramtui", 
				"185", 
				"1.0", 
				"ru", 
				"telegramtui", 
				"ru", 
				NULL, 
				NULL, 
				sendCode);

	tlo_t *invokeWithLayer = 
		tl_invokeWithLayer(
				185, initConnection);

	tlo_t *sentCode = tg_send(tg, invokeWithLayer); 
	if (!sentCode)
		return 1;

	printf("SENT CODE:\n");
	printf("\ttype: %.8x (%s)\n", sentCode->objs[1]->id, sentCode->objs[1]->name);
	printf("\tphone_code_hash: %s\n", sentCode->objs[2]->value.data);

	int code;
	printf("enter code: \n");
	scanf("%d", &code);
	printf("code: %d\n", code);
	char phone_code[32];
	sprintf(phone_code, "%d", code);

	tlo_t *signIn = tl_auth_signIn(
			phone, 
			(char *)(sentCode->objs[2]->value.data), 
			phone_code, 
			NULL);  
	
	tlo_t *authorization = tg_send(tg, signIn); 
	if (!authorization)
		return 1;

	printf("AUTHORIZATION:\n");
	buf_dump(authorization->value);


	return 0;
}
