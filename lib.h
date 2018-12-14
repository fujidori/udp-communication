#ifndef _CONTROL_SOCKET_H
#define _CONTROL_SOCKET_H

#define	BUFSIZE	1460

ssize_t pseudo_send(int s, uint8_t *buf, size_t len,
		struct sockaddr *to, socklen_t tolen);
ssize_t pseudo_recv(int s, uint8_t *buf, size_t len,
		struct sockaddr *from, socklen_t *fromlen);

#endif
