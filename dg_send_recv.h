#ifndef _DG_SEND_RECV_SOCKET_H
#define _DG_SEND_RECV_SOCKET_H

ssize_t dg_send_recv(int fd, const void *outbuff, size_t outbytes,
			 void *inbuff, size_t inbytes,
			 const struct sockaddr *destaddr, socklen_t destlen);

#endif
