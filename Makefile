CC=gcc
CFLAGS = -O2 -W -Wall -Wextra -Werror -Wmissing-prototypes -Wmissing-declarations

src = $(wildcard *.c)
obj = $(patsubst %.c, %.o, $(src))
header = $(wildcard *.h)

.PHONY: all
all: client server

server: $(obj) example/server.o example/utils.o
	$(CC) $(CFLAGS) $(obj) example/server.o example/utils.o -o server

client: $(obj) example/client.o example/utils.o
	$(CC) $(CFLAGS) $(obj) example/client.o example/utils.o -o client

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
	rm -f *.o example/*.o server client
