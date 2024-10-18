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
				"librg", 
				"185", 
				"1.0", 
				"ru", 
				"ru", 
				"ru", 
				NULL, 
				NULL, 
				sendCode);

	tg_send(tg, initConnection); 
	
	return 0;
}
