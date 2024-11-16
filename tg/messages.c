#include "tg.h"
#include "../tl/id.h"
#include "../mtx/include/net.h"
#include <stdlib.h>
#include "time.h"

tl_messages_dialogsSlice_t * tg_get_dialogs(tg_t *tg, void *on_err_data,
		void (*on_err)(void *on_err_data, tl_t *tl, const char *err))
{
	InputPeer inputPeer = tl_inputPeerSelf();
	buf_t getDialogs = 
		tl_messages_getDialogs(
				false,
			 	0, 
				time(NULL),
			 	0, 
				&inputPeer, 
				1,
			 	0);

	buf_dump(getDialogs);

	tl_t *tl = tl_send(getDialogs);

	if (tl && tl->_id == id_messages_dialogsSlice)
	{
		return (tl_messages_dialogsSlice_t*)tl;
	}

	//if (tl && tl->_id == id_messages_dialogs)
	//{
		//return (tl_messages_dialogs_t*)tl;
	//}

	// throw error
	char *err = tg_strerr(tl); 
	if (on_err)
		on_err(on_err_data, tl, err);
	free(err);
	// free tl
	/* TODO:  <14-11-24, kuzmich> */
	return NULL;
}
