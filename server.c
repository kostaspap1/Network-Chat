#include <stdio.h>
#include <unistd.h>
#include <sys/types.h> /* basic system data types */
#include <sys/socket.h> /* basic socket definitions */
#include <netinet/in.h> //for the internet domain addresses
#include <errno.h> /* for the EINTR constant */
#include <sys/wait.h> /* for the waitpid() system call */
#include <sys/un.h> /* for Unix domain sockets */
#include <stdlib.h>  //gia na mou douleuei to exit()
#include <signal.h> //for signal handler

#define MAXPENDING 5    /* Maximum outstanding connection requests */

/* global variables for signal handler */
int sockfd;                    		/* Socket descriptor gia ton server */
int sock1;	                    	/* Socket descriptor gia ton client 1*/
int sock2;	                    	/* Socket descriptor gia ton client 2*/

/* signal handler function for ctrl-c signals */
void handler_int() { //handler tou SIGINT
	printf ("\nUnexpected termination, closing all connections... \n");
	/* closing all connections */
	close(sockfd);
	close(sock1);
	close(sock2);
	exit(0);
}

int main(int argc, char *argv[])
{
	struct sockaddr_in servaddr; 		/* Local address */
	struct sockaddr_in clientaddr1; 	/* Client 1 address */
	struct sockaddr_in clientaddr2; 	/* Client 2 address */
	struct sockaddr_in client_udp1;		/* plhrofories tou prwtou client gia th udp sundesh */
	struct sockaddr_in client_udp2;		/* plhrofories tou deuterou client gia th udp sundesh */
	unsigned short servport;     		/* Server port */
	unsigned int clientlen1;            	/* Length of client 1 address data structure */
	unsigned int clientlen2;            	/* Length of client 2 address data structure */
	unsigned short num_cl=0;		/* Number of clients */
	int yes=1;				
	int cl1,cl2;				/* Random choice for udp client/server */

	if (argc != 2)     /* Test for correct number of arguments */
	{
		fprintf(stderr, "Usage:  %s <Server Port>\n", argv[0]);
		exit(0);
	}

	servport = atoi(argv[1]);  /* First arg:  local port */

	signal(SIGINT,handler_int); /* closing successfully programm with CTRL-C */

	/* Create socket for incoming connections */
	if ((sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		printf("Server couldn't create TCP socket properly. Start over.");

	/* Construct local address structure */
	memset(&servaddr, 0, sizeof(servaddr));   /* Zero out structure */
	servaddr.sin_family = AF_INET;                /* Internet address family */
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
	servaddr.sin_port = htons(servport);      /* Local port */

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
	    perror("setsockopt");
	    exit(1);
	}
	    /* Bind to the local address */
	if (bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
		printf("Server couldn't bind with the TCP Client.\n");

	    /* Mark the socket so it will listen for incoming connections */
	if (listen(sockfd, MAXPENDING) < 0)
		printf("Server couldn't \"listen\" to the TCP Client.");

	system ("clear");
	printf("*********************************************\n");
	printf("|Server is up and waiting for connections...|\n");
	printf("*********************************************\n\n");

	for (;;) /* Run forever */
	{
		/* Set the size of the in-out parameter */
		clientlen1 = sizeof(clientaddr1);
		/* Wait for a client to connect */
		printf("Server is waiting for a client...\n");	
		
		if ((sock1 = accept(sockfd, (struct sockaddr *) &clientaddr1, &clientlen1)) < 0)
		    printf("Server couldn't \"accept\" TCP Client.");
	
		num_cl++;
		printf("Client no. %d connected!\n", num_cl);

		clientlen2 = sizeof(clientaddr2);

		if ((sock2 = accept(sockfd, (struct sockaddr *) &clientaddr2, &clientlen2)) < 0)
		    printf("Server couldn't \"accept\" TCP Client.");
	
		num_cl++;
		printf("Client no. %d connected!\n", num_cl);
		/* lamvanei diadoxika tis duo structs me plhrofories apo tous clients */
		read( sock1, &client_udp1, sizeof(client_udp1) ); 
		read( sock2, &client_udp2, sizeof(client_udp2) ); 
	
		/*epilegei tuxaia poios ap tous clients 8a einai udp server
		kai poios udp client*/
		srand(time(0));
		cl1 = rand()%2;
		cl2 = cl1?0:1;

		//stelnei thn epilogh stous clients		
		write( sock1, &cl1 , sizeof(int) );
		write( sock2, &cl2 , sizeof(int) );
					
		//stelnei ston prwto client to struct me tis plhrofories tou deutero client
		write( sock1, &client_udp2 , sizeof(client_udp2) );
		
		//stelnei ston deutero client to struct me tis plhrofories tou prwto client
		write( sock2, &client_udp1 , sizeof(client_udp1) );

		//close sockets
		close(sock1);
		close(sock2);
    	}
    /* NOT REACHED */
}
