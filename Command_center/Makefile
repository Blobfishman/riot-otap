client: udp_client.c
	mkdir -p bin
	gcc udp_client.c -o ./bin/client

server: udp_server.c
	mkdir -p bin
	gcc udp_server.c -o ./bin/server

all: client server

clean:
	rm -rf ./bin
