/**
 * File              : net.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 21.11.2024
 * Last Modified Date: 22.11.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#include "../mtx/include/types.h"
#include "net.h"
#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <strings.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

int tl_net_open(const char * ip, uint32_t port)
{
  int sockfd;
  struct sockaddr_in serv_addr;
  struct hostent * server;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (0 > sockfd) {
		printf("can't open socket");
    return -1;
  }

  server = gethostbyname(ip);

  if (0 == server) {
    printf("no such host");
    return -1;
  }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr_list[0], (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(port);

  if (0 > connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) {
    printf("can't connect");
    return -1;
  }

	// send intermediate protocol
	char init[] = {0xee, 0xee, 0xee, 0xee};
	send(sockfd, init, 4, 0);

	shared_rc.net.sockfd = sockfd;
	shared_rc.seqnh = -1;

	return sockfd;
}

void tl_net_close(int sockfd)
{
  close(sockfd);
}

void tl_net_send(const buf_t buf)
{
  net_t net = shared_rc_get_net();
  i32_t n = (i32_t)send(net.sockfd, buf.data, buf.size, 0);

  if (n < 0) {
    printf("ERROR writing to socket");
  }
}

buf_t tl_net_receive()
{
	printf("net receive...\n");
	net_t net = shared_rc_get_net();
	buf_t buf;
	buf_init(&buf);
	unsigned long LEN = BUFSIZ;
	buf_t b;
	b.size = LEN;	
	do
	{		
			buf_init(&b);
			b.size = recv(net.sockfd,
				 	b.data, LEN, 0);
			if (b.size == 0)
			{
				printf("received nothing\n");
			}
			else if (b.size > 0)
			{
				buf = buf_cat(buf, b);
				printf("received %d bytes\n", b.size);
			}
			else /* < 0 */
			{
				fprintf(stderr, "socket error\n");
			}
			free(b.data);
	} while (b.size == LEN);

	printf("net receive ... done\n");
	buf_dump(buf);
  return buf;
}
