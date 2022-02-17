CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -pthread

all : server client

# Server Side
server : server.o server_main.o daemon.o ll_cache.o ll_int.o req_queue.o queue_int.o hashcache.o hashgraph.o bfs.o nans_threads.o DeInO.o
	$(CC) -o $@ $^ $(LDFLAGS)

server.o : server.c server_main.o DeInO.o
	$(CC) -c $(CFLAGS) $<

server_main.o : server_main.c server_main.h daemon.o queue_int.o nans_threads.o hashgraph.o DeInO.o
	$(CC) -c $(CFLAGS) $<

daemon.o : daemon.c daemon.h DeInO.o
	$(CC) -c $(CFLAGS) $<

nans_threads.o : nans_threads.c nans_threads.h req_queue.o queue_int.o hashcache.o hashgraph.o bfs.o DeInO.o
	$(CC) -c $(CFLAGS) $<


# Client Side
client : client.o client_main.o DeInO.o
	$(CC) -o $@ $^ $(LDFLAGS)

client.o : client.c client_main.o DeInO.o
	$(CC) -c $(CFLAGS) $<

client_main.o : client_main.c client_main.h DeInO.o
	$(CC) -c $(CFLAGS) $<


# Data Structures
ll_cache.o : data_structs/ll_cache.c data_structs/ll_cache.h
	$(CC) -c $(CFLAGS) $<

hashcache.o : data_structs/hashcache.c data_structs/hashcache.h ll_cache.o
	$(CC) -c $(CFLAGS) $<

ll_int.o : data_structs/ll_int.c data_structs/ll_int.h
	$(CC) -c $(CFLAGS) $<

hashgraph.o : data_structs/hashgraph.c data_structs/hashgraph.h ll_int.o
	$(CC) -c $(CFLAGS) $<

bfs.o : data_structs/bfs.c data_structs/bfs.h hashgraph.o ll_int.o
	$(CC) -c $(CFLAGS) $<

req_queue.o : data_structs/req_queue.c data_structs/req_queue.h
	$(CC) -c $(CFLAGS) $<

queue_int.o : data_structs/queue_int.c data_structs/queue_int.h
	$(CC) -c $(CFLAGS) $<


# Common
DeInO.o : DeLib/DeInO.c DeLib/DeInO.h
	$(CC) -c $(CFLAGS) $<

clean : 
	rm -f client server *.o