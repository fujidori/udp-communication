#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <inttypes.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "utils.h"
//#include "../lib.h"
#include "../ringbuf.h"

#define BUFSIZE (64 * 1024)
#define MAXLINE 1000
#define MSS 1460

static struct hdr {
  uint32_t	seq;	/* sequence # */
  uint32_t	ts;		/* timestamp when sent */
} recvhdr;
/* } sendhdr, recvhdr; */

int
main(int argc, char *argv[])
{
	char *port = "12345";
	char *filename = NULL;

	/*
	 * CLI option parser
	 */
	int option_index = 0;
	int c, err = 0;
	static struct option long_options[] = {
		{"port", required_argument, 0, 'p'},
		{"help", no_argument,       0,  0 },
		{0, 0, 0, 0}
	};
	static char usage[] = "usage: %s [-f filename] [-p port]\n";


	/* parse the command-line arguments */
	while ((c = getopt_long(argc, argv, "p:f:h", long_options, &option_index)) != -1) {
		switch (c) {
		case 0:	/* help */
			printf(usage, argv[0]);
			exit(EXIT_SUCCESS);

		case 'p':	/* port number */
			port = optarg;
			if (atoi(port) < 1024 || atoi(port) > 65535) {
				fprintf(stderr, "invalid port number: %s\n", optarg);
				err = 1;
			}
			break;

		case 'f':	/* file name */
			filename = optarg;
			break;

		case '?':
		default:
			err = 1;
			break;
		}
	}
	if (err || (optind < argc) || filename == NULL) {	/* error or extra arguments? */
		fprintf(stderr, usage, argv[0]);
		exit(EXIT_FAILURE);
	}


	/*
	 * Create and Bind socket
	 */
	struct addrinfo hints;
	struct addrinfo *res, *rp;
	int s, ret;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	ret = getaddrinfo(NULL, port, &hints, &res);
	if (ret != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
		exit(EXIT_FAILURE);
	}

	for (rp = res; rp != NULL; rp = rp->ai_next) {
		s = socket(rp->ai_family, rp->ai_socktype,
				rp->ai_protocol);
		if (s == -1)
			continue;

		if (bind(s, rp->ai_addr, rp->ai_addrlen) == 0)
			break;                  /* Success */

		close(s);
	}

	if (rp == NULL) {
		fprintf(stderr, "Could not bind\n");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(res);


	/*
	 * File Transfer
	 */
	uint8_t buf[BUFSIZE];
	struct ringbuf_t *rbuf = ringbuf_init(buf, BUFSIZE);
	ssize_t nread, nwrite;
	struct sockaddr from;
	socklen_t fromlen = sizeof(struct sockaddr);

	char msg[MAXLINE];

	FILE *fp;
	fp = fopen(filename, "w");
	if (fp == NULL) {
		perror("fopen");
		close(s);
		fclose(fp);
		exit(EXIT_FAILURE);
	}

	struct msghdr msgsend, msgrecv;
	memset(&msgsend, 0, sizeof(msgsend));
	memset(&msgrecv, 0, sizeof(msgrecv));
	struct iovec iovsend[2], iovrecv[2];

	while (1) {
		msgrecv.msg_name = &from;
		msgrecv.msg_namelen = fromlen;
		msgrecv.msg_iov = iovrecv;
		msgrecv.msg_iovlen = 2;
		iovrecv[0].iov_base = &recvhdr;
		iovrecv[0].iov_len = sizeof(struct hdr);
		iovrecv[1].iov_base = msg;
		iovrecv[1].iov_len = MAXLINE;

		nread = recvmsg(s, &msgrecv, 0);
		if (nread == -1){
			perror("recvmsg");
			close(s);
			fclose(fp);
			exit(EXIT_FAILURE);
		}

		msgsend.msg_name = &from;
		msgsend.msg_namelen = fromlen;
		msgsend.msg_iov = iovsend;
		msgsend.msg_iovlen = 2;
		iovsend[0].iov_base = &recvhdr;
		iovsend[0].iov_len = sizeof(struct hdr);
		iovsend[1].iov_base = msg;
		iovsend[1].iov_len = nread - sizeof(struct hdr);

		nwrite = sendmsg(s, &msgsend, 0);
		if (nwrite == -1){
			perror("sendmsg");
			close(s);
			fclose(fp);
			exit(EXIT_FAILURE);
		}

#ifdef DEBUG
		printf(" --- remote address --- \n");
		print_addrinfo(&from);
		printf(" --- received massage --- \n");
		printf("msg:%s\nmsglen:%zu\n", msg, sizeof(msg));
		printf(" --- read & write bytes --- \n");
		printf("nread:%zd\n", nread);
		printf("nwrite:%zd\n", nwrite);
		printf("\n");
#endif

		for (int i = 0; i < nread - (ssize_t)sizeof(struct hdr); i++) {
			ringbuf_push(rbuf, msg[i]);
			ringbuf_pop(rbuf, (uint8_t*)&msg[i]);
		}

		int r;
		r = fwrite(msg, nread, 1, fp);
		if (ferror(fp) == -1){
			fprintf(stderr, "fwrite\n");
			close(s);
			fclose(fp);
			exit(EXIT_FAILURE);
		}

		if (r) {
			break;
		}

		// if(c == EOF)
		// 	break;

		// for (int i = 0; i < nread; i++) {
		// 	if(fputc(msg[i], fp) == EOF)
		// 		break;
		// }

	}

	close(s);
	fclose(fp);
	return 0;

/***********************
#ifdef DEBUG
	struct sockaddr local;
	socklen_t local_len;
	getsockname(s, &local, &local_len);
	printf("server\nsocket:\n");
	print_addrinfo(&local);
#endif

	nread = pseudo_recv(s, buf, sizeof(buf), &from, &fromlen);
	if (nread == -1){
		fprintf(stderr, "Could not pseudo_recv()\n");
		close(s);
		exit(EXIT_FAILURE);
	}

#ifdef DEBUG
	printf("server\nreceive from:\n");
	print_addrinfo(&from);
	printf("server: received data: %s\n", buf);
#endif

	close(s);
	return 0;
*************************/
}
