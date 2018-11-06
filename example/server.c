#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <inttypes.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "utils.h"
#include "../lib.h"

int
main(int argc, char *argv[])
{
	char *port = "12345";

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
	static char usage[] = "usage: %s [-p port]\n";


	/* parse the command-line arguments */
	while ((c = getopt_long(argc, argv, "p:h", long_options, &option_index)) != -1) {
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

		case '?':
		default:
			err = 1;
			break;
		}
	}
	if (err || (optind < argc)) {	/* error or extra arguments? */
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
	 * Use lib
	 */
	uint8_t buf[BUFSIZE];
	ssize_t nread;
	struct sockaddr from;
	socklen_t fromlen;

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
}
