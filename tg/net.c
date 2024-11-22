#include <stdio.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <strings.h>

uint32_t net_open(const char *ip, uint32_t port)
{
  uint32_t sockfd;
  uint32_t error_code = 0;
  struct sockaddr_in serv_addr;
  struct hostent * server;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (0 > sockfd) {
    error_code = 1;

    goto process_error;
  }

  server = gethostbyname(ip);

  if (0 == server) {
    error_code = 2;

    goto process_error;
  }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr_list[0], (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(port);

  if (0 > connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) {
    error_code = 3;

    goto process_error;
  }

process_error:
   switch (error_code) {
     case 1:
     {
       printf("can't open socket");

       break;
     }
     case 2:
     {
       printf("no such host");

       break;
     }
     case 3:
     {
       printf("can't connect");

       break;
     }
     default:
     {
       break;
     }
   }

	 // send intermediate protocol
	 char init[] = {0xee, 0xee, 0xee, 0xee};
	 send(sockfd, init, 4, 0);

   return sockfd;
}

void net_close(uint32_t sockfd)
{
  close(sockfd);
}

void net_send(uint32_t sockfd, const buf_t buf)
{
	printf("net send...\n");
	buf_dump(buf);

  int32_t n = (int32_t)send(sockfd, buf.data, buf.size, 0);

  if (n < 0) {
    fprintf("ERROR writing to socket");
  }
	printf("done\n");
}

buf_t net_receive(uint32_t sockfd)
{
	printf("net receive...\n");
	buf_t buf;
	buf_init(&buf);
	unsigned long LEN = max_buf_size;
	buf_t b;
	b.size = LEN;	
	do
	{		
			buf_init(&b);
			b.size = recv(sockfd, b.data, LEN, 0);
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
