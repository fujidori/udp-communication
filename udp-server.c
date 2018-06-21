#include <sys/socket.h>	/* for socket(), bind() */
#include <sys/types.h>
#include <arpa/inet.h>	/* for sockaddr_in , inet_ntoa() */
#include <netdb.h>	/* for addrinfo */
#include <stdio.h>
#include <stdlib.h>	/* for EXIT_FAILURE */
#include <string.h>	/* for strlen(), menset() */
#include <getopt.h>	/* for getopt_long() */
#include <unistd.h>	/* for close() */
#include "shared.h"
#include "control_socket.h"

void receive_msg(char *port);

int
main(int argc, char *argv[])
{
	char *port = SERVICE_PORT;
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

	receive_msg(port);
	/* NOTREACHED */
}

void
receive_msg(char *port)
{
	struct addrinfo hints;
	struct addrinfo *res;
	int sfd, s;
	struct sockaddr_storage peer_addr;
	socklen_t peer_addr_len;
	char buf[BUFSIZE];	/* receive buffer */
	ssize_t recvlen;		/* bytes received */


	memset(&hints, 0, sizeof(struct addrinfo));
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

	sfd = bind_socket(&res);
	if(sfd == -1) {
		fprintf(stderr, "Could not bind_socket()\n");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(res);				/* No longer needed */


	/* now loop, receiving data and printing what we received */

	for (;;) {
		peer_addr_len = sizeof(struct sockaddr_storage);
		recvlen = recvfrom(sfd, buf, BUFSIZE, 0,
				(struct sockaddr *) &peer_addr, &peer_addr_len);
		if (recvlen == -1)
			continue;				/* Ignore failed request */
		buf[recvlen] = '\0';

		char host[NI_MAXHOST], service[NI_MAXSERV];

		s = getnameinfo((struct sockaddr *) &peer_addr, peer_addr_len,
						host, NI_MAXHOST, service, NI_MAXSERV, 
						NI_NUMERICHOST | NI_NUMERICSERV);
		if (s == 0) {
			printf("Received %zd bytes from %s:%s\n", recvlen, host, service);
		} else
			fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));
	}

	/* NOTREACHED */
	close(sfd);
}
