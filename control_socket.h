#ifndef _SET_SOCKET_H
#define _SET_SOCKET_H

#define	BUFSIZE	2048

int	bind_socket(struct addrinfo **res);
int	connect_socket(struct addrinfo **res);

int	send_msg(char *server, char *port);
void	recv_msg(char *port);

#endif
