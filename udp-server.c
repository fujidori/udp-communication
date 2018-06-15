#include <stdio.h>
#include <stdlib.h>	/* for EXIT_FAILURE */
#include <string.h>	/* for strlen(), menset() */
#include <getopt.h>	/* for getopt_long() */
#include <unistd.h>	/* for close() */
#include <sys/socket.h>	/* for socket(), bind() */
#include <arpa/inet.h>	/* for sockaddr_in , inet_ntoa() */
#include "shared.h"

void receive_msg(int port);

int
main(int argc, char *argv[])
{
	int port = SERVICE_PORT;
	int option_index = 0;
	int c, err = 0;
	static struct option long_options[] = {
		{"port",    required_argument, 0, 'p'},
		{"help",    no_argument,       0, 'h'},
		{0, 0, 0, 0}
	};
	static char usage[] = "usage: %s [-p port]\n";


	/* parse the command-line arguments */

	while ((c = getopt_long(argc, argv, "p:h", long_options, &option_index)) != -1) {
		switch (c) {
		case 0:	/* if flag(option's 3rd value) exits */
			printf("option %s", long_options[option_index].name);
			if (optarg)
				printf(" with arg %s has flag", optarg);
			printf("\n");
			break;

		case 'p':	/* port number */
			port = atoi(optarg);
			if (port < 1024 || port > 65535) {
				fprintf(stderr, "invalid port number: %s\n", optarg);
				err = 1;
			}
			break;

		case 'h':	/* help */
			printf(usage, argv[0]);
			exit(EXIT_SUCCESS);

		default:	/* '?' */
			err = 1;
			break;
		}
	}
	if (err || (optind < argc)) {	/* error or extra arguments? */
		fprintf(stderr, usage, argv[0]);
		exit(EXIT_FAILURE);
	}

	receive_msg(port);	/* never exits */
}

void
receive_msg(int port)
{
	struct sockaddr_in myaddr;
	struct sockaddr_in cliaddr;
	socklen_t cliaddr_len = sizeof(cliaddr);
	int fd;
	char buf[BUFSIZE];	/* receive buffer */
	ssize_t recvlen;		/* # bytes received */


	/* create a UDP socket */

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd == -1) {
		perror("cannot create socket\n");
		exit(EXIT_FAILURE);
	}

	/* bind the socket to any valid IP address and a specific port */

	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(port);

	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) == -1) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	/* now loop, receiving data and printing what we received */
	for (;;) {
		printf("waiting on port %d\n", port);
		recvlen = recvfrom(fd, buf, BUFSIZE, 0, (struct sockaddr *)&cliaddr, &cliaddr_len);
		printf("received %zd bytes\n", recvlen);
		if (recvlen > 0) {
			buf[recvlen] = '\0';
			printf("received message: \"%s\"\n", buf);
		}
	}
	/* never exits */
}
