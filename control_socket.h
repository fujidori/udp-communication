#ifndef _CONTROL_SOCKET_H
#define _CONTROL_SOCKET_H

#define	BUFSIZE	256

int	send_msg(char *server, char *port);
void	recv_msg(char *port);

#endif
