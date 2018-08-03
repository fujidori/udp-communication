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

struct hdr {
	struct sockaddr saddr;
	struct sockaddr daddr;
	socklen_t saddrlen;
	socklen_t daddrlen;
	uint32_t seq_num;	/* sequence number */
	uint32_t ack_num;	/* ack number */
	uint8_t	doff:5,
		fin:1,
		syn:1,
		ack:1;
	size_t dlen;	/* data length */
} __attribute__((packed));

static ssize_t send_data(int sockfd, struct hdr *hdr, uint8_t *data, size_t len);
static ssize_t recv_data(int sockfd, struct hdr *hdr, uint8_t *data, size_t len);
static int setsock(const char *server, const char *port, struct sockaddr *saddr,
		socklen_t *saddrlen, enum action act);


static int
setsock(const char *server, const char *port, struct sockaddr *saddr,
		socklen_t *saddrlen, enum action act)
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

		if ((*func[act])(sfd, rp->ai_addr, rp->ai_addrlen) == 0){
			if(saddr != NULL && saddrlen != NULL){
				memcpy(saddr, rp->ai_addr, sizeof(struct sockaddr));
				*saddrlen = rp->ai_addrlen;
			}
			break;					/* Success */
		}

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


/*
 * Send hdr and data
 */
static ssize_t
send_data(int sockfd, struct hdr *hdr, uint8_t *data, size_t len)
{
	struct iovec iov[2];
	int iovcnt;
	ssize_t bytes_write;

	hdr->dlen = len;

	iov[0].iov_base = hdr;
	iov[0].iov_len = sizeof(struct hdr);
	iov[1].iov_base = data;
	iov[1].iov_len = hdr->dlen;
	iovcnt = sizeof(iov) / sizeof(struct iovec);

	bytes_write = writev(sockfd, iov, iovcnt);
	if(bytes_write == -1) {
		perror("writev");
		return -1;
	}


	/* print sent data*/
	printf("   - Sent data -   \n");
	printf("hdr seq:%u, ack:%u\n", hdr->seq_num, hdr->ack_num);
	printf("whole data size: %zu bytes\n", bytes_write);
	printf("data size: %zu bytes\n", iov[1].iov_len);

	printf("bytes of data: ");
	for (size_t i = 0; i < hdr->dlen; i++)
		printf("%" PRIu8 " ", data[i]);
	printf("\n");

	return hdr->dlen;
}


/*
 * Receive data, separate hdr from data.
 * Return hdr, data and datalen
 */
static ssize_t
recv_data(int sockfd, struct hdr *hdr, uint8_t *data, size_t len)
{
	struct iovec iov[2];
	int iovcnt;
	ssize_t bytes_read;		/* bytes received */

	iov[0].iov_base = hdr;
	iov[0].iov_len = sizeof(struct hdr);
	iov[1].iov_base = data;
	iov[1].iov_len = len;
	iovcnt = sizeof(iov) / sizeof(struct iovec);

	bytes_read = readv(sockfd, iov, iovcnt);
	if(bytes_read == -1) {
		perror("readv");
		return -1;
	}

	data[hdr->dlen] = '\0';

	/* print received data*/
	printf("   - Received data -   \n");
	printf("hdr seq:%u, ack:%u\n", hdr->seq_num, hdr->ack_num);
	printf("whole data size: %zu bytes\n", bytes_read);
	printf("data size: %zu bytes\n", hdr->dlen);

	printf("bytes of data: ");
	for (size_t i = 0; i < hdr->dlen; i++) {
		printf("%" PRIu8 " ", data[i]);
	}
	printf("\n");

	return hdr->dlen;
}


/*
 * Send data, receive ack
 */
ssize_t
pseudo_send(char *server, char *port)
{
	/* data */
	uint8_t data = 'a';

	/* socket*/
	int sfd;	/* source fd */
	int dfd;	/* destination fd */
	uint8_t rbuf[BUFSIZE];
	ssize_t recvlen;
	ssize_t sendlen;

	struct sockaddr saddr;	/* source address */
	struct sockaddr daddr;	/* destination address */
	socklen_t saddrlen;
	socklen_t daddrlen;

	dfd = setsock(server, port, &daddr, &daddrlen, CONNECT);
	sfd = setsock(NULL, "55555", &saddr, &saddrlen, BIND);

	/* header */
	struct hdr shdr;	/* send hdr */
	shdr.seq_num = 0;
	shdr.ack_num = 0;
	shdr.dlen = sizeof(data);
	shdr.ack = 1;
	shdr.fin = 0;
	shdr.syn = 0;
	shdr.daddr = daddr;
	shdr.daddrlen = daddrlen;
	shdr.saddr = saddr;
	shdr.saddrlen = saddrlen;

	struct hdr rhdr;	/* received hdr */

	/* send data */
	printf("-----------------------\n");
	printf("Sending packet to %s:%s\n", server, port);
	sendlen = send_data(dfd, &shdr, &data, sizeof(data));
	if (sendlen == -1) {
		perror("send_data");
		close(sfd);
		close(dfd);
		return 1;
	}
	printf("Sent data: %c\n", data);


	/* receive and check ack */

	do{
		recvlen = recv_data(sfd, &rhdr, rbuf, sizeof(rbuf));
		if (recvlen == -1){
			fprintf(stderr, "Could not recv_data()\n");
			close(sfd);
			close(sfd);
			return -1;
		}
		printf("Received data: %s\n", rbuf);
	}while(rhdr.ack_num != shdr.seq_num + shdr.dlen);


	close(sfd);
	close(dfd);

	return sendlen;
}


/*
 * Receive hdr, send ack
 */
ssize_t
pseudo_recv(char *port, uint8_t *buf, ssize_t buflen)
{
	int sfd;	/* source fd */
	int dfd;	/* destination fd */
	struct sockaddr raddr;	/* receive sockaddr */
	socklen_t rlen;

	ssize_t recvlen;		/* bytes received */

	struct hdr rhdr;	/* receive hdr */
	struct hdr ack;	/* ack hdr */

	sfd = setsock(NULL, port, &raddr, &rlen, BIND);

	printf("-----------------------\n");
	recvlen = recv_data(sfd, &rhdr, buf, buflen);
	if (recvlen == -1){
		fprintf(stderr, "Could not recv_data()\n");
		close(sfd);
		return -1;
	}
	printf("Received data: %s\n", buf);

	ack.saddr = raddr;
	ack.daddr = rhdr.saddr;
	ack.ack = 0;
	ack.syn = 0;
	ack.fin = 0;
	ack.seq_num = rhdr.ack_num;
	ack.ack_num = rhdr.seq_num + rhdr.dlen;


	dfd = socket(rhdr.saddr.sa_family, SOCK_DGRAM, 0);
	if(connect(dfd, &rhdr.saddr, rhdr.saddrlen) == -1){
		perror("connect");
		close(sfd);
		close(dfd);
		return -1;
	}

	/* send ack */
	printf("  -- Sending ack --  \n");
	if(send_data(dfd, &ack, 0, 0) == -1){
		fprintf(stderr, "Could not send_ack()\n");
		close(sfd);
		close(dfd);
		return -1;
	}

	close(sfd);
	close(dfd);

	return recvlen;
}
