CC=gcc
DFLAGS=-g
CFLAGS=-lpthread -lm

build_server: server.c
	$(CC) -o server $(CFLAGS) $<

build_client: client.c
	$(CC) -o client $(CFLAGS) $<

debug_server: server.c
	$(CC) -o debug_server $(CFLAGS) $(DFLAGS) $<

debug_client: client.c
	$(CC) -o debug_client $(CFLAGS) $(DFLAGS) $<
	
clean: 
	rm -rf server client
