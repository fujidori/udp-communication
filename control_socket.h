#ifndef _CONTROL_SOCKET_H
#define _CONTROL_SOCKET_H

#define	BUFSIZE	256

ssize_t	pseudo_send(char *server, char *port);
ssize_t pseudo_recv(char *port, uint8_t *buf, ssize_t buflen);

#endif
