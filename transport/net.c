/**
 * File              : net.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 21.11.2024
 * Last Modified Date: 22.12.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#include "../tg/tg.h"
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include "../tg/list.h"
#include "../tl/alloc.h"
#include "queue.h"
#include "transport.h"

int tg_net_open_port(tg_t *tg, int port)
{
  struct sockaddr_in serv_addr;
  struct hostent * server;
	int sockfd;

  sockfd = 
		socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) {
		ON_ERR(tg, "%s: can't open socket", __func__);
    return -1;
  }

  server = gethostbyname(tg->ip);
 
  if (server == 0) {
		ON_ERR(tg, "%s: no host with ip: '%s'", __func__, tg->ip);
    return -1;
  }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy(
			(char *)server->h_addr_list[0],
		 	(char *)&serv_addr.sin_addr.s_addr,
		 	server->h_length);
  serv_addr.sin_port = htons(port);

  if (connect(
				sockfd, 
				(struct sockaddr *) &serv_addr, 
				sizeof(serv_addr)) < 0) 
	{
    ON_ERR(tg, "%s: can't connect", __func__);
    return -1;
  }

	// send intermediate protocol
	char init[] = {0xee, 0xee, 0xee, 0xee};
	send(sockfd, init, 4, 0);

	return sockfd;
}

int tg_net_open(tg_t *tg){
	return tg_net_open_port(tg, tg->port);
}

void tg_net_close(tg_t *tg, int sockfd)
{
  close(sockfd);
}

int tg_net_send(tg_t *tg, int sockfd, const buf_t buf)
{
  //ON_LOG_BUF(tg, buf, "%s: ", __func__);
  int32_t n = (int32_t)send(
			sockfd, 
			buf.data, 
			buf.size, 0);
  ON_LOG(tg, "%s: send size: %d", __func__, n);
  
  if (n < 0) {
    ON_ERR(tg, "%s: can't write to socket", __func__);
  }

	return n<0?1:0;
}

buf_t tg_net_receive(tg_t *tg, int sockfd)
{
	int LEN = BUFSIZ;
	buf_t data, buf;
	buf_init(&data);
	buf.size = LEN;	
	do
	{		
			buf_init(&buf);
			buf.size = recv(sockfd,
				 	buf.data, LEN, 0);
			if (buf.size == 0)
			{
				ON_LOG(tg, "%s: received nothing", __func__);
			}
			else if (buf.size > 0)
			{
				data = buf_cat(data, buf);
				ON_LOG(tg, "%s: received %d bytes", __func__, buf.size);
			}
			else /* < 0 */
			{
				ON_LOG(tg, "%s: socket error: %d", __func__, buf.size);
			}
			buf_free(buf);
			// give some time betwin packages
			usleep(100000); // in microseconds
	} while (buf.size == LEN);

	/*ON_LOG_BUF(tg, data, "%s: ", __func__);*/
  return data;
}

buf_t tg_net_queue_receive(tg_t *tg, int sockfd)
{
	buf_t buf;
	buf_init(&buf);

	// get length of the package
	uint32_t len;
	recv(sockfd, &len, 4, 0);
	ON_LOG(tg, "%s: prepare to receive len: %d", __func__, len);
	if (len < 0) {
		// this is error - report it
		ON_ERR(tg, "%s: received wrong length: %d", 
				__func__, len);
		return buf;
	}

	// realloc buf to be enough size
	if (buf_realloc(&buf, len)){
		// handle error
		ON_ERR(tg, "%s: error buf realloc to size: %d", 
				__func__, len);
		return buf;
	}

	// get data
	uint32_t received = 0; 
	while (received < len){
		received += recv(
				sockfd, 
				&buf.data[received], 
				len-received, 
				0);	
		ON_LOG(tg, "%s: received chunk: %d", __func__, received);
		
		if (received < 0){
			// some error
			ON_ERR(tg, "%s: socket error: %d", 
					__func__, received);
			return buf;
		}

		buf_t b = buf_add_buf(buf);
		tl_t *tl = tl_deserialize(&b);

		// ask to send new chunk
		//if (received < len){
			//if (chunk){
				//// get new chunk query
				//buf_t c = chunk(chunkp, received, len);
				//printf("CHUNK\n");
				//buf_dump(c);
				
				//buf_t buf = 
					//tg_prepare_query(tg, c, true, NULL);
				//buf_free(c);
				
				//// send
				//int s = 
					//send(sockfd, buf.data, buf.size, 0);
				//if (s < 0){
					//// handle send error
					//ON_ERR(tg, "%s: socket error: %d", 
							//__func__, s);
					//return buf;
				//}
				
				//// receive more data
				//continue;
			//}
			
			//ON_LOG(tg, "%s: expected size: %d, received: %d (%d%%)", 
					//__func__, len, received, received*100/len);
			
			//// receive more data
			//continue;
		//}
		break;
	}

	if (received < 0){
		// some error
		ON_ERR(tg, "%s: socket error: %d", 
				__func__, received);
		return buf;
	}

	// return buf
	buf.size = len;
	ON_LOG(tg, "%s: received: %s", __func__, buf_sdump(buf));
	return buf;
}


buf_t tg_net_receive2(tg_t *tg, int sockfd, void *chunkp,
		buf_t (*chunk)(void *chunkp, uint32_t received, uint32_t total))
{
	buf_t buf;
	buf_init(&buf);

	// get length of the package
	uint32_t len;
	recv(sockfd, &len, 4, 0);
	ON_LOG(tg, "%s: prepare to receive len: %d", __func__, len);
	if (len < 0) {
		// this is error - report it
		ON_ERR(tg, "%s: received wrong length: %d", 
				__func__, len);
		return buf;
	}

	// realloc buf to be enough size
	if (buf_realloc(&buf, len)){
		// handle error
		ON_ERR(tg, "%s: error buf realloc to size: %d", 
				__func__, len);
		return buf;
	}

	// get data
	uint32_t received = 0; 
	while (received < len){
		received += 
			recv(sockfd, &buf.data[received], len, 0);	
		ON_LOG(tg, "%s: received chunk: %d", __func__, received);
		
		if (received < 0){
			// some error
			ON_ERR(tg, "%s: socket error: %d", 
					__func__, received);
			return buf;
		}

		// ask to send new chunk
		if (received < len){
			if (chunk){
				// get new chunk query
				buf_t c = chunk(chunkp, received, len);
				printf("CHUNK\n");
				buf_dump(c);
				
				buf_t buf = 
					tg_prepare_query(tg, c, true, NULL);
				buf_free(c);
				
				// send
				int s = 
					send(sockfd, buf.data, buf.size, 0);
				if (s < 0){
					// handle send error
					ON_ERR(tg, "%s: socket error: %d", 
							__func__, s);
					return buf;
				}
				
				// receive more data
				continue;
			}
			
			ON_LOG(tg, "%s: expected size: %d, received: %d (%d%%)", 
					__func__, len, received, received*100/len);
			
			// receive more data
			continue;
		}
		break;
	}

	if (received < 0){
		// some error
		ON_ERR(tg, "%s: socket error: %d", 
				__func__, received);
		return buf;
	}

	// return buf
	buf.size = len;
	ON_LOG(tg, "%s: received: %s", __func__, buf_sdump(buf));
	return buf;
}

int tg_net_add_query(tg_t *tg, const buf_t buf, uint64_t msg_id, 
		void *on_donep, int (*on_done)(void *on_donep, const buf_t buf),
		void *chunkp, 
		buf_t (*chunk)(void *chunkp, uint32_t received, uint32_t total))
{
	printf("%s\n", __func__);

	tg_queue_node_t *n = 
		tg_queue_node_new(buf, msg_id, 
				on_donep, on_done, chunkp, chunk);
	if (!n){
		ON_ERR(tg, "tg_queue_node_new error");
		return 1;
	}

	// wait to queue unlock
	while (tg->send_queue_lock) {
		usleep(1000); // in microseconds
	}
	tg->send_queue_lock = 1;
	list_append(&tg->send_queue, n,
			ON_ERR(tg, "list_append"); 
			tg->send_queue_lock = 0;
			return 1);
	
	tg->send_queue_lock = 0;
	return 0;
}

int tg_net_send_queue_node(tg_t *tg){
	//printf("%s\n", __func__);
	int ret = 1, sockfd = -1;
	while (tg->send_queue_lock) {
		printf("%s: queue is locked...\n", __func__);
		usleep(1000); // in microseconds
	}
	tg->send_queue_lock = 1;
	
	tg_queue_node_t *n = list_remove(&tg->send_queue, 0); 
	tg->send_queue_lock = 0;
	if (n){
		// open socket
		sockfd = tg_net_open(tg);
		if (sockfd < 0){
			goto tg_net_send_queue_node_finish;
		}

		// send ACK
		if (tg->msgids[0]){
			buf_t ack = tg_ack(tg);
			send(sockfd, ack.data, ack.size, 0);
			buf_free(ack);
		}

		// send query
		ON_LOG(tg, "%s: send: %s", __func__, buf_sdump(n->msg));
		int s = -1;
		while (s < 0){
			s = send(sockfd, n->msg.data,
			 	n->msg.size, 0);
			if (s < 0){
				// handle error
				ON_ERR(tg, "%s: socket error: %d", 
						__func__, s);
				if (n->on_done){
					// if user return non-null and s < 0 try to send again
					if(n->on_done(n->on_donep, buf_new())){
						usleep(100000); // in microseconds
						continue;
					}
				}
				goto tg_net_send_queue_node_finish;
			}
		}

		// receive data
		while (1) {
			buf_t buf = tg_net_receive2(
					tg, sockfd, n->chunkp, n->chunk);
			
			if (buf.size > 0)
				ret = 0;
			
			if (n->on_done){
				if (n->on_done(n->on_donep, buf)){
					// receive again
					continue;
				};	
			}
			
			break;
		}
	}
	
tg_net_send_queue_node_finish:;
	if (n){
		tg_queue_node_free(n);
	}
	if (sockfd >= 0)
		tg_net_close(tg, sockfd);
	return ret;
}
