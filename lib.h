#ifndef _CONTROL_SOCKET_H
#define _CONTROL_SOCKET_H

#define	BUFSIZE	256

ssize_t pseudo_send(int s, void *buf, size_t len,
		struct sockaddr *to, socklen_t tolen);
ssize_t pseudo_recv(int s, void *buf, size_t len,
		struct sockaddr *from, socklen_t *fromlen);

#endif
