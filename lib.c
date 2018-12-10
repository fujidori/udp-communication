#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include "lib.h"
#include "example/utils.h"

struct hdr {
	uint32_t seq_num;	/* sequence number */
	uint32_t ack_num;	/* ack number */
	uint8_t	doff:5,
		fin:1,
		syn:1,
		ack:1;
} __attribute__((packed));

struct pbuf {
	struct pbuf *next;
	void *payload;
	uint16_t len;
	uint16_t tot_len;
};

static ssize_t send_pbuf(int s, struct hdr *hdr, struct pbuf *pbuf,
		struct sockaddr *to, socklen_t tolen);
static ssize_t recv_pbuf(int s, struct hdr *hdr, struct pbuf *pbuf,
		struct sockaddr *from, socklen_t *fromlen);
static void init_hdr(struct hdr *hdr);
//static int send_syn();
//static int send_synack();
//static int send_ack();

#ifdef DEBUG
static void
print_pbuf(struct hdr *hdr, struct pbuf *pbuf)
{
	printf("seq:%u\n", hdr->seq_num); 
	printf("ack:%u\n", hdr->ack_num);
	//printf("data: %zu bytes\n", hdr->dlen);
	printf("bytes of data: ");
	for (size_t i = 0; i < pbuf->len; i++)
		printf("%" PRIu8 " ", ((uint8_t *)(pbuf->payload))[i]);
	printf("\n");
	return;
}
#endif

static void 
init_hdr(struct hdr *hdr)
{
	hdr->seq_num = 0;
	hdr->ack_num = 0;
	hdr->ack = 0;
	hdr->fin = 0;
	hdr->syn = 0;
	return;
}

static ssize_t
send_pbuf(int s, struct hdr *hdr, struct pbuf *pbuf,
		struct sockaddr *to, socklen_t tolen)
{
	struct iovec iov[2];
	ssize_t nwrite;

	struct msghdr msg;

	msg.msg_name = to;
	msg.msg_namelen = tolen;
	msg.msg_iov = iov;
	msg.msg_iovlen = sizeof(iov) / sizeof(struct iovec);
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	iov[0].iov_base = hdr;
	iov[0].iov_len = sizeof(struct hdr);
	iov[1].iov_base = pbuf;
	iov[1].iov_len = pbuf->len;

	nwrite = sendmsg(s, &msg, 0);
	if(nwrite == -1) {
		perror("send_data: sendmsg");
		return -1;
	}

#ifdef DEBUG
	/* print sent data*/
	printf("--------------------\n");
	printf("   - Sent data -   \n");
	printf("to ");
	print_addrinfo(to);
	print_pbuf(hdr, pbuf);
	printf("segment size: %zu bytes\n", nwrite);
	printf("--------------------\n");
#endif

	return pbuf->len;
}


static ssize_t
recv_pbuf(int s, struct hdr *hdr, struct pbuf *pbuf,
		struct sockaddr *from, socklen_t *fromlen)
{
	struct iovec iov[2];
	ssize_t nread;

	int ret;
	struct sockaddr src;
	socklen_t srclen = sizeof(struct sockaddr_in);
	ret = getsockname(s, &src, &srclen);
	if(ret == -1){
		perror("getsockname");
		return -1;
	}
	printf("recv_data socket:\n");
	print_addrinfo(&src);


	struct msghdr msg;
	memset(&msg,0,sizeof(msg));
	msg.msg_name = from;

	if(fromlen != NULL)
		msg.msg_namelen = *fromlen;
	else
		msg.msg_namelen = 0;

	msg.msg_iov = iov;
	msg.msg_iovlen = sizeof(iov) / sizeof(struct iovec);
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	iov[0].iov_base = hdr;
	iov[0].iov_len = sizeof(struct hdr);
	iov[1].iov_base = pbuf;
	iov[1].iov_len = pbuf->len;

	nread = recvmsg(s, &msg, 0);
	if(nread == -1) {
		perror("recv_data: recvmsg");
		return -1;
	}

	printf("recv_data from:\n");
	print_addrinfo(from);

	//pbuf[hdr->dlen] = '\0';

#ifdef DEBUG
	/* print received data*/
	printf("--------------------\n");
	printf("   - Received data -   \n");
	printf("from ");
	print_addrinfo((struct sockaddr *)from);
	print_pbuf(hdr, pbuf);
	printf("segment size: %zu bytes\n", nread);
	printf("--------------------\n");
#endif

	return pbuf->len;
}


/*
 * Send data, receive ack
 */
ssize_t
pseudo_send(int s, void *buf, size_t len, struct sockaddr *to, socklen_t tolen)
{
	// uint8_t rbuf[BUFSIZE];
	ssize_t nread, nwrite;
	
	struct pbuf pbuf;
	pbuf.payload = buf;
	pbuf.len = len;

	struct sockaddr from;
	socklen_t fromlen;
	memset(&from, 0, sizeof(from));
	fromlen = 0;


	/* header */
	struct hdr shdr;	/* send hdr */
	init_hdr(&shdr);

	struct hdr rhdr;	/* received hdr */

	/* send data */
	nwrite = send_pbuf(s, &shdr, &pbuf, to, tolen);
	if (nwrite == -1) {
		fprintf(stderr, "Could not send_data()\n");
		return -1;
	}

	/* receive and check ack */
	// do{
		nread = recv_pbuf(s, &rhdr, &pbuf, &from, &fromlen);
		if (nread == -1){
			fprintf(stderr, "Could not recv_data()\n");
			return -1;
		}
		printf("received ack\n");
		// printf("data: %s\n", (uint8_t)pbuf.payload);
	// }while(rhdr.ack_num != shdr.seq_num + shdr.dlen);

	return nwrite;
}


/*
 * Receive hdr, send ack
 */
ssize_t
pseudo_recv(int s, void *buf, size_t len, struct sockaddr *from, socklen_t *fromlen)
{
	struct hdr rhdr;	/* receive hdr */
	struct hdr ack;	/* ack hdr */
	ssize_t nread;
	struct pbuf pbuf;
	pbuf.len = len;


	nread = recv_pbuf(s, &rhdr, &pbuf, from, fromlen);
	if (nread == -1){
		fprintf(stderr, "Could not recv_data()\n");
		return -1;
	}
	memcpy(buf, pbuf.payload, pbuf.len);
	// buf = pbuf.payload;

	ack.ack = 0;
	ack.syn = 0;
	ack.fin = 0;
	ack.seq_num = rhdr.ack_num;
	ack.ack_num = rhdr.seq_num + pbuf.len;

	struct pbuf pbuf_empty;
	/* send ack */
	if(send_pbuf(s, &ack, &pbuf_empty, from, *fromlen) == -1){
		fprintf(stderr, "Could not send_ack()\n");
		return -1;
	}

	return nread;
}
