#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include "utils.h"

void 
print_addrinfo(struct sockaddr *addr)
{

	if (addr == NULL) {
		return;
	}

	char addrbuf[INET6_ADDRSTRLEN];
	switch(addr->sa_family){
	case AF_INET:
		printf("\tAF_INET\n");
		printf("\t%s:%d\n", 
				inet_ntop(addr->sa_family, 
					&((struct sockaddr_in *)addr)->sin_addr,
					addrbuf, sizeof(addrbuf)),
				ntohs(((struct sockaddr_in *)addr)->sin_port));
		break;
	case AF_INET6:
		printf("AF_INET6\n");
		printf("\t%s:%d\n", 
				inet_ntop(addr->sa_family, 
					&((struct sockaddr_in6 *)addr)->sin6_addr,
					addrbuf, sizeof(addrbuf)),
				ntohs(((struct sockaddr_in6 *)addr)->sin6_port));
		break;
	case AF_UNSPEC:
		printf("AF_UNSPEC\n");
		break;
	default:
		printf("unknown address family\n");
		break;
	}

	return;
}

// int
// getaddr(const char *host, const char *port, struct sockaddr *addr,
// 		socklen_t *addrlen)
// {
// 	struct addrinfo hints;
// 	struct addrinfo *res;
// 	int s;
//
// 	memset(&hints, 0, sizeof(hints));
// 	hints.ai_family = AF_UNSPEC;
// 	hints.ai_socktype = SOCK_DGRAM;
// 	// hints.ai_protocol = 0;
// 	hints.ai_flags = AI_PASSIVE;
//
// 	s = getaddrinfo(host, port, &hints, &res);
// 	if (s != 0) {
// 		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
// 		return -1;
// 	}
//
// 	if(addr != NULL && addrlen != NULL){
// 		memcpy(addr, res->ai_addr, sizeof(struct sockaddr));
// 		*addrlen = res->ai_addrlen;
// 	}
//
// 	freeaddrinfo(res);				/* No longer needed */
//
// 	return 0;
// }

// int
// set_socket(const char *host, const char *port, struct sockaddr *addr,
// 		socklen_t *addrlen, enum action act)
// {
// 	struct addrinfo hints;
// 	struct addrinfo *res;
// 	struct addrinfo *rp;
// 	int sfd, s;
// 	int (*func[])(int sockfd, const struct sockaddr *addr, socklen_t addrlen) = {
// 		bind,
// 		connect,
// 	};
//
// 	memset(&hints, 0, sizeof(hints));
// 	hints.ai_family = AF_UNSPEC;
// 	hints.ai_socktype = SOCK_DGRAM;
// 	hints.ai_protocol = 0;
//
// 	switch(act) {
// 	case BIND:
// 		hints.ai_flags = AI_PASSIVE;
// 		break;
// 	case CONNECT:
// 		hints.ai_flags = 0;
// 		break;
// 	default:
// 		hints.ai_flags = 0;
// 	}
//
// 	s = getaddrinfo(host, port, &hints, &res);
// 	if (s != 0) {
// 		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
// 		return -1;
// 	}
//
//
// 	/*
// 	 * Try each address until we successfully bind() or connect().
// 	 * If socket() (or bind(), connect()) fails, we (close the socket
// 	 * and) try the next address.
// 	 */
//
// 	for (rp = res; rp != NULL; rp = rp->ai_next) {
// 		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
// 		if (sfd == -1)
// 			continue;
//
// 		if ((*func[act])(sfd, rp->ai_addr, rp->ai_addrlen) == 0){
// 			if(addr != NULL && addrlen != NULL){
// 				memcpy(addr, rp->ai_addr, sizeof(struct sockaddr));
// 				*addrlen = rp->ai_addrlen;
// 			}
// 			break;					/* Success */
// 		}
//
// 		close(sfd);
// 	}
//
// 	if (rp == NULL) {				/* No address succeeded */
// 		fprintf(stderr, "No address\n");
// 		return -1;
// 	}
//
// 	if(sfd == -1) {
// 		fprintf(stderr, "Could not bind or connect socket\n");
// 		close(sfd);
// 		return -1;
// 	}
//
// 	freeaddrinfo(res);				/* No longer needed */
//
// 	return sfd;
// }
