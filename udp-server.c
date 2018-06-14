#include "shared.h"

void
main(int argc, char const* argv[])
{
	struct sockaddr_in myaddr;
	struct sockaddr_in cliaddr;
	socklen_t cliaddr_len = sizeof(cliaddr);
	int fd;
	uint16_t port = SERVICE_PORT;
	char buf[BUFSIZE];	/* receive buffer */
	int recvlen;			/* # bytes received */


	/* create a UDP socket */

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cannot create socket\n");
		exit(EXIT_FAILURE);
	}

	/* bind the socket to any valid IP address and a specific port */

	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(port);

	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	/* now loop, receiving data and printing what we received */
	for (;;) {
		printf("waiting on port %d\n", port);
		recvlen = recvfrom(fd, buf, BUFSIZE, 0, (struct sockaddr *)&cliaddr, &cliaddr_len);
		printf("received %d bytes\n", recvlen);
		if (recvlen > 0) {
			buf[recvlen] = '\0';
			printf("received message: \"%s\"\n", buf);
		}
	}
	/* never exits */
}
