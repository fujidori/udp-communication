#include <stdio.h>
#include <stdlib.h>	/* for EXIT_FAILURE */
#include <string.h>	/* for strlen(), menset() */
#include <getopt.h>	/* for getopt_long() */
#include <netdb.h>	/* for gethostbyname() */ /* now not used */
#include <unistd.h>	/* for close() */
#include <sys/socket.h>	/* for inet_ntoa() */
#include <arpa/inet.h>	/* for inet_ntoa() */
#include <netinet/in.h>	/* for inet_ntoa() */

#define SERVICE_ADDR	"127.0.0.1"	/* hard-coded ip address */
#define SERVICE_PORT	12345	/* hard-coded port number */
#define BUFSIZE	2048

