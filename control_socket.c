#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
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

static int set_socket(const char *server, char *port, enum action act);
static int send_data(int sockfd, uint8_t *data, size_t len);
static int recv_entry(int sockfd, uint8_t *data, size_t len);

struct hdr {
	uint32_t seq;	/* sequence number */
	uint32_t ack;	/* ack number */
	size_t dlen;	/* data length */
} __attribute__((packed));


static int set_socket(const char *server, char *port, enum action act)
{
	struct addrinfo hints;
	struct addrinfo *res;
	struct addrinfo *rp;
	int sfd, s;
	int (*func[])(int sockfd, const struct sockaddr *addr, socklen_t addrlen) = {
		bind,
		connect,
	};

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = 0;

	switch(act) {
	case BIND:
		hints.ai_flags = AI_PASSIVE;
		break;
	case CONNECT:
		hints.ai_flags = 0;
		break;
	default:
		hints.ai_flags = 0;
	}

	s = getaddrinfo(server, port, &hints, &res);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}


	/*
	 * Try each address until we successfully bind() or connect().
	 * If socket() (or bind(), connect()) fails, we (close the socket
	 * and) try the next address.
	 */

	for (rp = res; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1)
			continue;

		if ((*func[act])(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
			break;					/* Success */

		close(sfd);
	}

	if (rp == NULL) {				/* No address succeeded */
		fprintf(stderr, "No address\n");
		return -1;
	}

	if(sfd == -1) {
		fprintf(stderr, "Could not bind or connect socket\n");
		close(sfd);
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(res);				/* No longer needed */

	return sfd;
}

static int send_data(int sockfd, uint8_t *data, size_t len)
{
	struct hdr hdr;

	int iovcnt;
	ssize_t bytes_write;
	struct iovec iov[2];

	hdr.seq = 0;
	hdr.ack = 0;
	hdr.dlen = len;

	iov[0].iov_base = &hdr;
	iov[0].iov_len = sizeof(struct hdr);
	iov[1].iov_base = data;
	iov[1].iov_len = hdr.dlen;
	iovcnt = sizeof(iov) / sizeof(struct iovec);

	bytes_write = writev(sockfd, iov, iovcnt);
	if(bytes_write == -1) {
		perror("writev");
		return(-1);
	}


	/* print sent data*/

	printf("   - Sent data -   \n");
	printf("hdr seq:%u, ack:%u\n", hdr.seq, hdr.ack);
	printf("whole data size: %zu bytes\n", bytes_write);
	printf("data size: %zu bytes\n", iov[1].iov_len);

	printf("bytes of data: ");
	for (size_t i = 0; i < hdr.dlen; i++)
		printf("%" PRIu8 " ", data[i]);
	printf("\n");

	return hdr.dlen;
}

static int recv_entry(int sockfd, uint8_t *data, size_t len)
{
	struct hdr hdr;
	struct iovec iov[2];
	int iovcnt;
	ssize_t bytes_read;		/* bytes received */

	iov[0].iov_base = &hdr;
	iov[0].iov_len = sizeof(struct hdr);
	iov[1].iov_base = data;
	iov[1].iov_len = len;
	iovcnt = sizeof(iov) / sizeof(struct iovec);

	bytes_read = readv(sockfd, iov, iovcnt);
	if(bytes_read == -1) {
		perror("readv");
		return(-1);
	}

	data[hdr.dlen] = '\0';


	/* print received data*/

	printf("   - Received data -   \n");
	printf("hdr seq:%u, ack:%u\n", hdr.seq, hdr.ack);
	printf("whole data size: %zu bytes\n", bytes_read);
	printf("data size: %zu bytes\n", hdr.dlen);

	printf("bytes of data: ");
	for (size_t i = 0; i < hdr.dlen; i++) {
		printf("%" PRIu8 " ", data[i]);
	}
	printf("\n");

	return hdr.dlen;
}

int
send_msg(char *server, char *port)
{
	int sfd;
	uint8_t data = 'a';

	sfd = set_socket(server, port, CONNECT);

	/* send the messages */

	for (;;) {
		printf("-----------------------\n");
		printf("Sending packet to %s:%s\n", server, port);
		if (send_data(sfd, &data, sizeof(data)) == -1) {
			perror("send_data");
			close(sfd);
			return 1;
		}
		printf("Sent data: %c\n", data);
	}

	close(sfd);
	return 0;
}

void
recv_msg(char *port)
{
	int sfd;
	uint8_t sbuf[BUFSIZE];
	ssize_t recvlen;		/* bytes received */

	sfd = set_socket(NULL, port, BIND);

	/* now loop, receiving data and printing what we received */

	for (;;) {
		printf("-----------------------\n");
		recvlen = recv_entry(sfd, sbuf, sizeof(sbuf));
		if (recvlen == -1)
			continue;				/* Ignore failed request */
		printf("Received data: %s\n", sbuf);
	}

	/* NOTREACHED */
	close(sfd);
}
