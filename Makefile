all: server client

server: server.c 
	gcc -w server.c -o server

client: client.c 
	gcc -w client.c -o client
