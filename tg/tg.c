#include "../libtg.h"
#include "../tl/tl.h"
#include "../mtx/include/api.h"
#include "../tl/alloc.h"
#include "../mtx/include/net.h"
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

struct tg_ {
	app_t app;
	int apiId;
	char apiHash[32];
	sqlite3 *db;
};

tg_t *tg_new(const char *database_path,
		int apiId, const char apiHash[32])
{
	if (!database_path)
		return NULL;

	// allocate struct
	tg_t *tg = NEW(tg_t, return NULL);	
	
	// connect to SQL
	int err = sqlite3_open_v2(
			database_path, &tg->db, 
			SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 
			NULL);
	if (err){
		api.log.error((char *)sqlite3_errmsg(tg->db));
		return NULL;
	}

	// set apiId and apiHash
	tg->apiId = apiId;
	memcpy(tg->apiHash, apiHash, 32);
  
	// start mtproto
	tg->app = api.app.open();
	api.log.info(".. new session");
  shared_rc.ssid = api.buf.rand(8);
	api.srl.ping();

	return tg;
}

void tg_close(tg_t *tg)
{
	// close Telegram
	
	// close mtproto
  net_t n = shared_rc_get_net();
  api.net.close(n);
	
	// close database
	sqlite3_close(tg->db);
	
	// free
	free(tg);
}

buf_t parse_answer(buf_t a)
{
  ui32_t id = api.buf.get_ui32(a);
  param_t p;
  p.id = id;
  buf_t s = buf_add(a.data + 4, a.size - 4); // hack
  //api.buf.dump(s);
  printf("current id: %.8x\n", id);
  switch (id) {
    case id_pong:
    {
      ctor_Pong_t c;
      c.id__ = id;
      c.type__ = API;
      p.value = s;
      p.type = TYPE_LONG;
      c.msg_id = api.sel.deserialize_param(p);
      //api.buf.dump(c.msg_id.value);
      s = api.buf.add(s.data + 8, s.size - 8);
      p.value = s;
      p.type = TYPE_LONG;
      c.ping_id = api.sel.deserialize_param(p);
      //api.buf.dump(c.ping_id.value);
      //t.ctor_Pong = c;
      break;
    }
		case id_msgs_ack:
    {
/* A server usually acknowledges the receipt of a message
 * from a client (normally, an RPC query) using an RPC
 * response. If a response is a long time coming, a
 * server may first send a receipt acknowledgment, and
 * somewhat later, the RPC response itself. */
      
			ui64_t *msg_id = (ui64_t *)(s.data);
      printf("msgs_ack: %.16lx\n", *msg_id);
			
			// waight for message
			buf_t r = net_receive();
      //api.buf.dump(r);

			buf_t tr = api.trl.detransport(r);
      //api.buf.dump(tr);
      
			buf_t d = api.enl.decrypt(tr, API);
      //api.buf.dump(d);
      
			buf_t s1r = api.hdl.deheader(d, API);
      //api.buf.dump(s1r);
			
			parse_answer(s1r);

      break;
    }
		case id_rpc_result:
		{
			ui64_t msg_id = *(ui64_t *)(s.data);
      printf("rpc_result msg_id: %.16lx, ", msg_id);
			ui32_t *object_id = (ui32_t *)(s.data + 8);
      printf("object_id: %.8x\n", *object_id);
			
			switch (*object_id) {
				case id_rpc_error:
				{
					//RPC Error
					ctor_rpc_error_t c;
					c.id__ = *object_id;
					c.type__ = RFC;
					
					s = api.buf.add(s.data + 12, s.size - 12);
					p.value = s;
					p.type = TYPE_INT;
					c.error_code = api.sel.deserialize_param(p);
					
					s = api.buf.add(s.data + 4, s.size - 4);
					p.value = s;
					p.type = TYPE_STRING;
					c.error_message = api.sel.deserialize_param(p);

					printf("rpc_error_code: %.8x, ", 
							*(int*)(c.error_code.value.data));
					printf("rpc_error_message: %s\n", 
							c.error_message.value.data);
					
					buf_t buf = {};
					memset(&buf, 0, sizeof(buf));
					return buf;
				}
				default: break;
			}
			s = buf_add(s.data + 8, s.size - 8);
			return s;
			break;
		}
    case id_bad_msg_notification:
    {
      printf("current id: %.8x\n", id);
			long *id = (long *)(s.data);
			int *seqn = (int *)(s.data + 8);
			int *code = (int *)(s.data + 12);
      api.log.info(
					"id_bad_msg_notification received");
      printf(
					"ID: %0.16lx "
					"SQN: %d " 
					"CODE: %d\n", 
					 *id, *seqn, *code);

      break;
    }
    default:
    {
      printf("current id: %.8x\n", id);
      break;
    }
  }
  return a;
}


tl_t * tg_send(tg_t *tg, buf_t s)
{
	api.srl.ping();
	printf("Send:\n");
	buf_dump(s);

	buf_t s1 = api.hdl.header(s, API);
  /*api.buf.dump(s1);*/
  buf_t e = api.enl.encrypt(s1, API);
  /*api.buf.dump(e);*/
  buf_t t = api.trl.transport(e);
  /*api.buf.dump(t);*/
  buf_t nr = api.net.drive(t, SEND_RECEIVE);
	/*api.buf.dump(nr);*/
      
	buf_t tr = api.trl.detransport(nr);
  /*api.buf.dump(tr);*/
	buf_t d = api.enl.decrypt(tr, API);
  /*api.buf.dump(d);*/
  buf_t s1r = api.hdl.deheader(d, API);
  /*api.buf.dump(s1r);*/

	buf_t a = parse_answer(s1r);
	printf("Answer:\n");
	buf_dump(a);
	
	// acknowledge message
	//api.srl.msgsAck(*a.data + 4);

	// deserialize message
	return tl_deserialize(&a);
}
