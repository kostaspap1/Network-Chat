#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <signal.h>	/* library for signal handler */

/*global variables for signal handler*/
int sock,sockfd;         /* Socket descriptors */


/* signal handler function for ctrl-c signals */
void handler_int() { //handler tou SIGINT
	printf ("\nUnexpected termination, closing all connections... \n");
	/* closing all connections */
	close(sock);
	close(sockfd);
	exit(0);
}

int main(int argc, char *argv[])
{
	
	struct sockaddr_in servaddr; 	 	/* server address */
	struct sockaddr_in mine; 		/* plhrofories tou trexontos client gia th udp sundesh */
	struct sockaddr_in other; 		/* plhrofories tou allou client gia th udp sundesh */
	unsigned short servport,udpport;     	/* server port, udp port */
	char *servIP;                    	/* Server IP address (dotted quad) */
	char str[INET_ADDRSTRLEN];		/* IP in type string */
	int guess;				/* Random choice for udp client/server(from server) */
	struct ifreq ifr;			/* domh gia na pairnoume thn ip tou client */
	char iface[] = "eth0";
	/* udp server/client statements */
	int n;
	struct sockaddr_in servaddr_udp,cliaddr_udp;
	socklen_t len;
	char mesg[1000];	
	char sendline[1000];

	if (argc!=4)    /* Test for correct number of arguments */
	{
		fprintf(stderr, "Usage: %s <Server IP> <TCP port> <UDP port>\n", argv[0]);
		exit(1);
	}

	signal(SIGINT,handler_int); /* closing successfully programm with CTRL-C */

	servIP = argv[1];             /* server IP address (dotted quad) */

	servport = atoi(argv[2]); 	/* servers port */

	udpport = atoi(argv[3]);	/* input udp port */

	/* Create a reliable, stream socket using TCP */
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		printf("Client couldn't create TCP socket properly. Start over.");

	/* Construct the server address structure */
	memset(&servaddr, 0, sizeof(servaddr));     /* Zero out structure */
	servaddr.sin_family      = AF_INET;             /* Internet address family */
	servaddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
	servaddr.sin_port        = htons(servport); /* Server port */

	//Type of address to retrieve
	ifr.ifr_addr.sa_family = AF_INET;
	
	//Copy the interface name in the ifreq structure
	strncpy(ifr.ifr_name, iface, IFNAMSIZ-1);
	ioctl(sock, SIOCGIFADDR, &ifr);	
	inet_ntop(AF_INET,  &((struct sockaddr_in *)&ifr.ifr_addr )->sin_addr, str, INET_ADDRSTRLEN);

	//bazoume thn IP tou trexontos client kai th udp port tou sth struct gia na th steiloume
	mine.sin_addr.s_addr 	= inet_addr(str);   /* IP */
	mine.sin_port 		= htons(udpport); /* udp port */
	mine.sin_family 	= AF_INET;

	/* Establish the connection to the server */
	if (connect(sock, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
		printf("Client couldn't connect to the Server. Start over.\n");

	/* stelnei sto server struct me ts plhrofories tou trexontos client */
	write(sock, &mine, sizeof(mine));

	system ("clear");
	printf("Waiting for another client to connect... \n");
	
	//diabazei thn epilogh tou server gia to an 8a einai udp_client h udp_sever
	if(read(sock, &guess, sizeof(int))<0){
		printf("Server closed unexpextedly!\n");
		close(sock);
		exit(0);
	}
	printf("Connection is successful\n");
	printf("%s\n", guess?"You are the UDP Server":"You are the UDP Client");

	//pairnei plhrofories gia ton allo client
	read(sock, &other, sizeof(other) ); 

	close(sock);//kleinei to tcp socket
						
	if(guess){	/* UDP SERVER MODE */

		sockfd=socket(AF_INET, SOCK_DGRAM,0);

		bzero(&servaddr_udp, sizeof(servaddr_udp));
		servaddr_udp.sin_family = AF_INET;
		servaddr_udp.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
		servaddr_udp.sin_port = mine.sin_port; //kataxwrei th port pou tou edwse o xrhsths

		if (bind(sockfd, (struct sockaddr*)&servaddr_udp, sizeof(servaddr_udp))>0)
			printf("UDP Server couldn't bind with the UDP Client.\n");

		printf("You now wait for the UDP Client to contact:\n");
		
		for (;;)
		{
			len = sizeof(cliaddr_udp);
			/* dexetai minimata apo ton udp client */
			n = recvfrom(sockfd,mesg,1000,0,(struct sockaddr *)&cliaddr_udp,&len);
			mesg[n] = 0;
			printf("\nReceived the following from the UDP Client:\n");
			printf("%s",mesg);
		}
	
	} else {				/* UDP CLIENT MODE */
		sockfd=socket(AF_INET,SOCK_DGRAM,0);

		bzero(&servaddr, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = other.sin_addr.s_addr;//bazei thn ip tou tcp client
		servaddr.sin_port = other.sin_port;//bazei thn port tou udp server

		printf("You can now contact the UDP Server:\n>>");

		while (fgets(sendline, 10000, stdin) != NULL)
		{
			/* stelnei minimata ston udp server */

			if(sendto(sockfd, sendline, strlen(sendline), 0, (struct sockaddr *)&servaddr, sizeof(servaddr))<0)
				printf("sending message failed\n");	
			else printf("You said: %s>>",sendline);
		}	
	}
}
