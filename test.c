#include "libtg.h"
#include "mtx/include/buf.h"
#include "mtx/include/types.h"
#include "tl/deserialize.h"
#include "tl/id.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{

	char phone[32];
	printf("enter phone number (+7XXXXXXXXXX): \n");
	scanf("%s", phone);
	printf("phone number: %s\n", phone);

	tg_t *tg = tg_new("test.db", 24646404, "818803c99651e8b777c54998e6ded6a0");

	buf_t codeSettings = tl_codeSettings(
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

	buf_t sendCode = 
		tl_auth_sendCode(
				phone, 
				24646404, 
				"818803c99651e8b777c54998e6ded6a0", 
				&codeSettings);

	buf_t initConnection = 
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
				&sendCode);

	buf_t invokeWithLayer = 
		tl_invokeWithLayer(
				185, &initConnection);

	tl_auth_sentCode_t *sentCode = 
		(tl_auth_sentCode_t *)tg_send(tg, invokeWithLayer); 
	if (!sentCode || sentCode->_id != TL_ID_auth_sentCode)
		return 1;

	printf("SENT CODE:\n");
	printf("\ttype: %.8x (%s)\n", sentCode->arg_type->_id, "");
	printf("\tphone_code_hash: %s\n", sentCode->arg_phone_code_hash);

	int code;
	printf("enter code: \n");
	scanf("%d", &code);
	printf("code: %d\n", code);
	char phone_code[32];
	sprintf(phone_code, "%d", code);

	buf_t signIn = tl_auth_signIn(
			phone, 
			sentCode->arg_phone_code_hash, 
			phone_code, 
			NULL);  
	
	tl_authorization_t *authorization = 
		(tl_authorization_t *)tg_send(tg, signIn); 
	if (!authorization || authorization->_id != TL_ID_authorization)
		return 1;

	printf("AUTHORIZATION: %d\n", authorization->_id);

	return 0;
}
