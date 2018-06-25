#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>	/* for sockaddr_in, inet_ntoa() */
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "control_socket.h"

int
bind_socket(struct addrinfo **res)
{
	int fd;
	struct addrinfo *rp;

	/*
	 * Try each address until we successfully bind().
	 * If socket() (or bind()) fails, we (close the socket
	 * and) try the next address.
	 */

	for (rp = *res; rp != NULL; rp = rp->ai_next) {
		fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (fd == -1)
			continue;

		if (bind(fd, rp->ai_addr, rp->ai_addrlen) == 0)
			break;					/* Success */

		close(fd);
	}

	if (rp == NULL) {				/* No address succeeded */
		fprintf(stderr, "Could not bind\n");
		return -1;
	}

	return fd;
}

int
connect_socket(struct addrinfo **res)
{
	int fd;
	struct addrinfo *rp;

	// ----------------------------------------
	/*
	 * Try each address until we successfully connect().
	 * If socket() (or connect()) fails, we (close the socket
	 * and) try the next address.
	 */

	for (rp = *res; rp != NULL; rp = rp->ai_next) {
		fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (fd == -1)
			continue;
	
		if (connect(fd, rp->ai_addr, rp->ai_addrlen) != -1)
			break;					/* Success */
	
		close(fd);
	}
	
	if (rp == NULL) {				/* No address succeeded */
		fprintf(stderr, "Could not connect\n");
		return -1;
	}

	return fd;
}

int
send_msg(char *server, char *port)
{
	struct addrinfo hints;
	struct addrinfo *res;
	int sfd, s;
	char buf[BUFSIZE] = "This packet is from client";

	memset(&hints, 0, sizeof(hints));
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

void
recv_msg(char *port)
{
	struct addrinfo hints;
	struct addrinfo *res;
	int sfd, s;
	struct sockaddr_storage peer_addr;
	socklen_t peer_addr_len;
	char buf[BUFSIZE];	/* receive buffer */
	ssize_t recvlen;		/* bytes received */


	memset(&hints, 0, sizeof(hints));
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