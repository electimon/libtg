#include "tg.h"
#include <string.h>

char *
tg_strerr(tl_t *tl)
{
	if (tl == NULL)
		return strdup("OBJECT_IS_NULL");

	char *str = NULL;

	switch (tl->_id) {
		case id_rpc_error:
			{
				tl_rpc_error_t *err = 
					(tl_rpc_error_t *)tl;
				str = strdup((char *)err->error_message_.data);
			}
			break;
		
		case id_bad_msg_notification:
			{
				tl_bad_msg_notification_t *bmsgn = 
						(tl_bad_msg_notification_t *)tl;
				char err[256];
				sprintf(err, "BAD_MSG_NOTIFICATION: %d",
						bmsgn->error_code_);
				str = strdup(err);
			}
			break;
		
		default:
			str = strdup("UNKNOWN_ERROR");
			break;
	}
	return str;
}
