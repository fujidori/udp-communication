#include "shared.h"

int
main(int argc, char *argv[])
{
	extern char *optarg;
	extern int optind;
	int c, err = 0;
	static char usage[] = "usage: %s [-a serveraddr] [-p port]\n";

	struct sockaddr_in servaddr;
	socklen_t servaddr_len  = sizeof(servaddr);
	int fd;
	char *server = SERVICE_ADDR;
	int port = SERVICE_PORT;
	char buf[BUFSIZE] = "This packet is from client";


	/* parse the command-line arguments */

	while ((c = getopt(argc, argv, "a:p:")) != -1)
		switch (c) {
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
		default:	/* '?' */
			err = 1;
			break;
		}
	if (err || (optind < argc)) {	/* error or extra arguments? */
		fprintf(stderr, usage, argv[0]);
		exit(EXIT_FAILURE);
	}

	/* create a UDP socket */

	if ( (fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

    /* Filling server information */

	memset((char *) &servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	if (inet_aton(server, &servaddr.sin_addr) == 0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(EXIT_FAILURE);
	}

	/* send the messages */

	printf("Sending packet to %s port %d\n", server, port);
	if (sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&servaddr, servaddr_len) == -1)
		perror("sendto");

	close(fd);
	return 0;
}
