#include "libtg.h"
#include "tg/tl_object.h"
#include <stdbool.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
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
				"+79990407731", 
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
	printf("SENT CODE:\n");
	printf("\ttype: %.8x (%s)\n", sentCode->objs[1]->id, sentCode->objs[1]->name);
	printf("\tphone_code_hash: %s\n", sentCode->objs[2]->value.data);
	
	return 0;
}
