CC=gcc
CFLAGS = -O2 -W -Wall -Wextra -Werror -Wmissing-prototypes -Wmissing-declarations

src = $(wildcard *.c)
obj = $(patsubst %.c, %.o, $(src))
header = $(wildcard *.h)

.PHONY: all
all: client server ft-server ft-client

server: $(obj) example/server.o
	$(CC) $(CFLAGS) $(obj) example/server.o -o server

client: $(obj) example/client.o
	$(CC) $(CFLAGS) $(obj) example/client.o -o client

ft-server: $(obj) example/ft-server.o
	$(CC) $(CFLAGS) $(obj) example/ft-server.o -o ft-server

ft-client: $(obj) example/ft-client.o
	$(CC) $(CFLAGS) $(obj) example/ft-client.o -o ft-client

example/%.o: example/%.c example/%.h
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c ${headers}
	$(CC) $(CFLAGS) -c $< -o $@

debug:
	make all "CFLAGS=-g3 -O0 -W -Wall -Wextra -Werror -Wmissing-prototypes -Wmissing-declarations -DDEBUG"

# .SUFFIXES: .c .o
# .c.o:
# 	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f *.o example/*.o server client ft-server ft-client
