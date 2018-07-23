#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>	/* for sockaddr_in, inet_ntoa() */
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include "control_socket.h"

enum action {
	BIND = 0,
	CONNECT
};

static int set_socket(struct addrinfo **res, enum action act);
static int send_data(int sockfd, uint8_t *data, size_t len);
static int recv_all(int sockfd, uint8_t *data, size_t len);

struct hdr {
	uint32_t seq;
	uint32_t ack;
} __attribute__((packed));


static int
set_socket(struct addrinfo **res, enum action act)
{
	int fd;
	struct addrinfo *rp;
	int (*func[])(int sockfd, const struct sockaddr *addr, socklen_t addrlen) = {
		bind,
		connect,
	};


	/*
	 * Try each address until we successfully bind() or connect().
	 * If socket() (or bind(), connect()) fails, we (close the socket
	 * and) try the next address.
	 */

	for (rp = *res; rp != NULL; rp = rp->ai_next) {
		fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (fd == -1)
			continue;

		if ((*func[act])(fd, rp->ai_addr, rp->ai_addrlen) == 0)
			break;					/* Success */

		close(fd);
	}

	if (rp == NULL) {				/* No address succeeded */
		fprintf(stderr, "Could not bind\n");
		return -1;
	}

	return fd;
}

static int send_data(int sockfd, uint8_t *data, size_t len)
{
	u_int8_t *ptr;
	u_int8_t sbuf[BUFSIZE];
	struct hdr *hdr;

	ptr = sbuf;
	hdr = (struct hdr *)ptr;
	memset(hdr,0,sizeof(struct hdr));
	hdr->seq = 0;
	hdr->ack = 0;

	ptr+=sizeof(struct hdr);
	memcpy(ptr,data,len);
	ptr+=len;

	write(sockfd, sbuf, ptr-sbuf);
	printf("%ld bytes\n",ptr-sbuf);

	return 0;
}

static int recv_all(int sockfd, uint8_t *data, size_t len)
{
	struct hdr *hdr;
	uint8_t sbuf[BUFSIZE];
	u_int8_t *ptr = sbuf;
	size_t plen;	// payload length
	plen = len;
	hdr=(struct hdr *)ptr;
	ptr+=sizeof(struct hdr);
	plen-=sizeof(struct hdr);

	ssize_t recvlen;		/* bytes received */

	recvlen = read(sockfd, sbuf, sizeof(sbuf));

	/* print header*/
	printf("hdr seq:%u, ack:%u\n", hdr->seq, hdr->ack);
	/* print received whole data*/
	for (int i = 0; i <recvlen ; i++) {
		printf("Received data: ");
		printf("%" PRIu8 "\n", sbuf[i]);
	}

	memcpy(data, ptr, plen);

	return plen;

}

int
send_msg(char *server, char *port)
{
	struct addrinfo hints;
	struct addrinfo *res;
	int sfd, s;
	uint8_t data = 'a';

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;

	s = getaddrinfo(server, port, &hints, &res);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		return 1;
	}

	sfd = set_socket(&res, CONNECT);	/* connect socket */
	if(sfd == -1) {
		fprintf(stderr, "Could not connect_socket()\n");
		return 1;
	}

	freeaddrinfo(res);				/* No longer needed */


	/* send the messages */

	for (;;) {
		printf("-----------------------\n");
		printf("Sending packet to %s:%s\n", server, port);
		if (send_data(sfd, &data, sizeof(data)) == -1) {
			perror("send_data");
			close(sfd);
			return 1;
		}
	}

	close(sfd);
	return 0;
}

void
recv_msg(char *port)
{
	struct addrinfo hints;
	struct addrinfo *res;
	int sfd, s;
	uint8_t sbuf[BUFSIZE];
	ssize_t recvlen;		/* bytes received */

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	s = getaddrinfo(NULL, port, &hints, &res);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	sfd = set_socket(&res, BIND);	/* bind socket */
	if(sfd == -1) {
		fprintf(stderr, "Could not bind_socket()\n");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(res);				/* No longer needed */


	/* now loop, receiving data and printing what we received */

	for (;;) {
		printf("-----------------------\n");
		recvlen = recv_all(sfd, sbuf, sizeof(sbuf));
		if (recvlen == -1)
			continue;				/* Ignore failed request */
	}

	/* NOTREACHED */
	close(sfd);
}
