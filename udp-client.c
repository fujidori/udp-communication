#include "shared.h"

int
main(int argc, char const* argv[])
{
	struct sockaddr_in servaddr;
	socklen_t servaddr_len  = sizeof(servaddr);
	int fd;
	char *server = SERVICE_ADDR;
	uint16_t port = SERVICE_PORT;
	char buf[BUFSIZE] = "This packet is from client";

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
