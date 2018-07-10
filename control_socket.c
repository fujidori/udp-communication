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

enum action {
	BIND = 0,
	CONNECT
};

static int set_socket(struct addrinfo **res, enum action act);

struct UDP_DATA {
	int seq_num;
	char data[BUFSIZE];
};

static int
set_socket(struct addrinfo **res, enum action act)
{
	int fd;
	struct addrinfo *rp;
	int (*func[])(int sockfd, const struct sockaddr *addr, socklen_t addrlen) = {
		bind,
		connect,
	};


	/*
	 * Try each address until we successfully bind() or connect().
	 * If socket() (or bind(), connect()) fails, we (close the socket
	 * and) try the next address.
	 */

	for (rp = *res; rp != NULL; rp = rp->ai_next) {
		fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (fd == -1)
			continue;

		if ((*func[act])(fd, rp->ai_addr, rp->ai_addrlen) == 0)
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
send_msg(char *server, char *port)
{
	struct addrinfo hints;
	struct addrinfo *res;
	int sfd, s;
	struct UDP_DATA udata;
	udata.seq_num = 0;
	strcpy(udata.data, "This packet is from client");

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

	sfd = set_socket(&res, CONNECT);	/* connect socket */
	if(sfd == -1) {
		fprintf(stderr, "Could not connect_socket()\n");
		return 1;
	}

	freeaddrinfo(res);				/* No longer needed */


	/* send the messages */

	for (;;) {
		printf("Sending packet to %s:%s\n", server, port);
		if (send(sfd, &udata, sizeof(udata), 0) == -1) {
			perror("sendto");
			close(sfd);
			return 1;
		}
		udata.seq_num++;
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
	int recvcount;
	struct sockaddr_storage peer_addr;
	socklen_t peer_addr_len;
	struct UDP_DATA udata;		/* receive buffer */
	ssize_t recvlen;		/* bytes received */
	recvcount = 0;

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

	sfd = set_socket(&res, BIND);	/* bind socket */
	if(sfd == -1) {
		fprintf(stderr, "Could not bind_socket()\n");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(res);				/* No longer needed */


	/* now loop, receiving data and printing what we received */

	for (;;) {
		peer_addr_len = sizeof(struct sockaddr_storage);
		recvlen = recvfrom(sfd, &udata, sizeof(udata), 0,
					(struct sockaddr *) &peer_addr, &peer_addr_len);
		if (recvlen == -1)
			continue;				/* Ignore failed request */
		udata.data[recvlen] = '\0';

		char host[NI_MAXHOST], service[NI_MAXSERV];

		s = getnameinfo((struct sockaddr *) &peer_addr, peer_addr_len,
						host, NI_MAXHOST, service, NI_MAXSERV, 
						NI_NUMERICHOST | NI_NUMERICSERV);
		if (s == 0) {
			printf("Received %zd bytes, seq_num:%d from %s:%s\n",
					recvlen, udata.seq_num, host, service);
			recvcount++;
		} else
			fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));

		if(udata.seq_num % 1000 == 0)
			printf("Loss data:%lf%%\n",
					100 * (1 - ((double)recvcount/((double)udata.seq_num + 1))));
	}

	/* NOTREACHED */
	close(sfd);
}
