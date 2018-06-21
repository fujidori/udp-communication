#include <sys/socket.h>	/* for socket() */
#include <sys/types.h>
#include <arpa/inet.h>	/* for sockaddr_in, inet_ntoa() */
#include <netdb.h>	/* for addrinfo */
#include <stdio.h>
#include <stdlib.h>	/* for EXIT_FAILURE */
#include <string.h>	/* for strlen(), memset() */
#include <getopt.h>	/* for getopt_long() */
#include <unistd.h>	/* for close() */
#include "shared.h"
#include "control_socket.h"

int send_msg(char *server, char *port);

int
main(int argc, char *argv[])
{
	char *server = SERVICE_ADDR;
	char *port = SERVICE_PORT;
	int option_index = 0;
	int c, err = 0;
	static struct option long_options[] = {
		{"port", required_argument, 0, 'p'},
		{"host", required_argument, 0, 'h'},
		{"help", no_argument,       0,  0 },
		{0, 0, 0, 0}
	};
	static char usage[] = "usage: %s [-h host] [-p port]\n";


	/* parse the command-line arguments */

	while ((c = getopt_long(argc, argv, "h:p:", long_options,
			&option_index)) != -1) {
		switch (c) {
		case 0:	/* help */
			printf(usage, argv[0]);
			exit(EXIT_SUCCESS);

		case 'h':	/* host */
			server = optarg;
			break;
	
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

	if (send_msg(server, port))
		exit(EXIT_FAILURE);
	return 0;
}

int
send_msg(char *server, char *port)
{
	struct addrinfo hints;
	struct addrinfo *res;
	int sfd, s;
	char buf[BUFSIZE] = "This packet is from client";

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;

	s = getaddrinfo(server, port, &hints, &res);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		return 1;
	}

	sfd = connect_socket(&res);
	if(sfd == -1) {
		fprintf(stderr, "Could not connect_socket()\n");
		return 1;
	}

	freeaddrinfo(res);				/* No longer needed */


	/* send the messages */

	printf("Sending packet to %s port %s\n", server, port);
	if (send(sfd, buf, strlen(buf), 0) == -1) {
		perror("sendto");
		return 1;
	}

	close(sfd);
	return 0;
}
