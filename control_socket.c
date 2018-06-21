#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdio.h>
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
