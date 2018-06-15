#include <stdio.h>
#include <stdlib.h>	/* for EXIT_FAILURE */
#include <string.h>	/* for strlen(), memset() */
#include <getopt.h>	/* for getopt_long() */
#include <unistd.h>	/* for close() */
#include <sys/socket.h>	/* for socket() */
#include <arpa/inet.h>	/* for sockaddr_in, inet_ntoa() */
#include "shared.h"

int send_msg(char *server, int port);

int
main(int argc, char *argv[])
{
	char *server = SERVICE_ADDR;
	int port = SERVICE_PORT;
	int option_index = 0;
	int c, err = 0;
	static struct option long_options[] = {
		{"address", required_argument, 0, 'a'},
		{"port",    required_argument, 0, 'p'},
		{"help",    no_argument,       0, 'h'},
		{0, 0, 0, 0}
	};
	static char usage[] = "usage: %s [-a serveraddr] [-p port]\n";


	/* parse the command-line arguments */

	while ((c = getopt_long(argc, argv, "a:p:h", long_options, &option_index)) != -1) {
		switch (c) {
		case 0:	/* if flag(option's 3rd value) exits */
			printf("option %s", long_options[option_index].name);
			if (optarg)
				printf(" with arg %s has flag", optarg);
			printf("\n");
			break;

		case 'a':	/* server address */
			server = optarg;
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

	if (send_msg(server, port))
		exit(EXIT_FAILURE);
	return 0;
}

int
send_msg(char *server, int port)
{
	struct sockaddr_in servaddr;
	socklen_t servaddr_len  = sizeof(servaddr);
	int fd;
	char buf[BUFSIZE] = "This packet is from client";


	/* create a UDP socket */

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd == -1) {
		perror("socket creation failed");
		return 1;
	}

    /* Filling server information */

	memset((char *) &servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	if (inet_aton(server, &servaddr.sin_addr) == 0) {
		fprintf(stderr, "inet_aton() failed\n");
		return 1;
	}

	/* send the messages */

	printf("Sending packet to %s port %d\n", server, port);
	if (sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&servaddr, servaddr_len) == -1) {
		perror("sendto");
		return 1;
	}

	close(fd);
	return 0;
}
