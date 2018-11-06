#ifndef _UTILS_H
#define _UTILS_H

#include <sys/socket.h>

enum action {
	BIND = 0,
	CONNECT
};

// int set_socket(const char *host, const char *port, struct sockaddr *addr,
// 		socklen_t *addrlen, enum action act);
// int getaddr(const char *host, const char *port, struct sockaddr *addr,
// 		socklen_t *addrlen);
void print_addrinfo(struct sockaddr *addr);

#endif
