#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "utils.h"
//#include "../lib.h"
#include "../ringbuf.h"

#define BUFSIZE 1460

int
main(int argc, char *argv[])
{
	char *host = "localhost";
	char *port = "12345";
	char *filename = NULL;

	/*
	 * CLI option parser
	 */
	int option_index = 0;
	int c, err = 0;
	static struct option long_options[] = {
		{"port", required_argument, 0, 'p'},
		{"host", required_argument, 0, 'h'},
		{"help", no_argument,       0,  0 },
		{0, 0, 0, 0}
	};
	static char usage[] = "usage: %s [-f filename] [-h host] [-p port]\n";


	/* parse the command-line arguments */
	while ((c = getopt_long(argc, argv, "h:p:f:", long_options,
			&option_index)) != -1) {
		switch (c) {
		case 0:	/* help */
			printf(usage, argv[0]);
			exit(EXIT_SUCCESS);

		case 'h':	/* host */
			host = optarg;
			break;
	
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
	if (err || (optind < argc || filename == NULL)) {	/* error or extra arguments? */
		fprintf(stderr, usage, argv[0]);
		exit(EXIT_FAILURE);
	}


	/*
	 * Create socket
	 */
	struct addrinfo hints;
	struct addrinfo *res, *rp;
	int s, ret;

	struct sockaddr to;	/* destination address */
	socklen_t tolen;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	ret = getaddrinfo(host, port, &hints, &res);
	if (ret != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
		exit(EXIT_FAILURE);
	}

	for (rp = res; rp != NULL; rp = rp->ai_next) {
		s = socket(rp->ai_family, rp->ai_socktype,
					rp->ai_protocol);
		if (s == -1)
			continue;

		memcpy(&to, rp->ai_addr, sizeof(struct sockaddr));
		tolen = rp->ai_addrlen;
		break;
	}

	if (rp == NULL) {
		close(s);
		fprintf(stderr, "Could not socket\n");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(res);


	/*
	 * File Transfer
	 */
	uint8_t buf[BUFSIZE];
	struct ringbuf_t *sbuf = ringbuf_init(buf, BUFSIZE);

	FILE *fp;
	fp = fopen(filename, "r");
	if (fp == NULL) {
		perror("fopen");
		fclose(fp);
		exit(EXIT_FAILURE);
	}

	while (1) {
		char c = fgetc(fp);
		ringbuf_push(sbuf, c);

#ifdef DEBUG
		printf("%c\n", c);
#endif

		ringbuf_pop(sbuf, (uint8_t*)&c);
		sendto(s, &c, sizeof(c), 0, &to, tolen);

		if(feof(fp)) {
			break;
		}
	}

	close(s);
	fclose(fp);
	return 0;

/**************************
	uint8_t data[] = "aaa";

#ifdef DEBUG
	printf("client\nsend to:\n");
	print_addrinfo(&to);
	printf("\tsend data: %s\n", data);
#endif

	if (pseudo_send(s, data, sizeof(data), &to, tolen) == -1){
		fprintf(stderr, "Could not pseudo_send()\n");
		close(s);
		exit(EXIT_FAILURE);
	}

	close(s);
	return 0;
**************************/
}
