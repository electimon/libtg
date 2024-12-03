/**
 * File              : net.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 21.11.2024
 * Last Modified Date: 03.12.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#include "../tg/tg.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

int tg_net_open_port(tg_t *tg, int port)
{
  struct sockaddr_in serv_addr;
  struct hostent * server;
	int sockfd;

  sockfd = 
		socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) {
		ON_ERR(tg, NULL, "%s: can't open socket", __func__);
    return 1;
  }

  server = gethostbyname(tg->ip);
 
  if (server == 0) {
		ON_ERR(tg, NULL, "%s: no host with ip: '%s'", __func__, tg->ip);
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
    ON_ERR(tg, NULL, "%s: can't connect", __func__);
    return 1;
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

void tg_net_send(tg_t *tg, int sockfd, const buf_t buf)
{
  //ON_LOG_BUF(tg, buf, "%s: ", __func__);
  int32_t n = (int32_t)send(
			sockfd, 
			buf.data, 
			buf.size, 0);
  ON_LOG(tg, "%s: send size: %d", __func__, n);
  
  if (n < 0) {
    ON_LOG(tg, "%s: can't write to socket", __func__);
  }
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
