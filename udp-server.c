#include <sys/socket.h>	/* for socket(), bind() */
#include <sys/types.h>
#include <arpa/inet.h>	/* for sockaddr_in, inet_ntoa() */
#include <netdb.h>	/* for addrinfo */
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>	/* for EXIT_FAILURE */
#include <string.h>	/* for strlen(), memset() */
#include <getopt.h>	/* for getopt_long() */
#include <unistd.h>	/* for close() */
#include "default.h"
#include "control_socket.h"

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

	recv_msg(port);
	/* NOTREACHED */
}
