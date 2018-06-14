CC=gcc
CFLAGS = -g

.PHONY: all
all: udp-server udp-client

udp-server: udp-server.o
	$(CC) $(CFLAGS) -o udp-server udp-server.o

udp-client: udp-client.o
	$(CC) $(CFLAGS) -o udp-client udp-client.o

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm -f udp-server udp-client *.o 
