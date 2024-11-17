#include "tg.h"
#include "../tl/id.h"
#include "../mtx/include/net.h"
#include <stdlib.h>
#include "time.h"

void tg_get_dialogs(tg_t *tg, int off_msg_id, int limit,
		void *data,
		int (*callback)(void *data, 
			tl_messages_dialogsSlice_t *dialogs, const char *err))
{
	int i, id=off_msg_id;
	long hash = 0;
	InputPeer inputPeer = tl_inputPeerSelf();
	time_t t = time(NULL);

	/*for (i = 0; i < limit; ++i) {*/
	for (i = 0; i < 1; ++i) {
		printf("ID: %d\n", id);
		buf_t getDialogs = 
			tl_messages_getDialogs(
					true,
					0, 
					time(NULL),
					id, 
					&inputPeer, 
					2,
					hash);

		/*buf_dump(getDialogs);*/

		tl_t *tl = tl_send(getDialogs);

		if (tl && tl->_id == id_messages_dialogsSlice)
		{
			if (callback)
				if (callback(data, (tl_messages_dialogsSlice_t *)tl, NULL))
					break;;
			tl_messages_dialogsSlice_t *md = 
				(tl_messages_dialogsSlice_t *)tl;
			
			int k;
			for (k = 0;  k< md->messages_len; ++k) {
				tl_message_t *m = (tl_message_t *)md->messages_[k];
				hash = hash ^ (hash >> 21);
				hash = hash ^ (hash << 35);
				hash = hash ^ (hash >> 4);
				hash = hash + m->id_;
				id = m->id_;
			}
		} else {
			// throw error
			char *err = tg_strerr(tl); 
			if (callback)
				if(callback(data, (tl_messages_dialogsSlice_t *)tl, err))
					break;
			free(err);
		}
	}
}
