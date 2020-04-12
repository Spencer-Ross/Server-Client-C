/***********************************************************************
name: Spencer Ross
    assignment8 -- Server-Client
description:    This program can start in one of two states: (1) server
				(2) client. If the program is in server mode, it will
				setup a socket, bind it, and listen for clients. Once a
				client is connected, it will write the date in time to
				the client, and output the client's host name and number
				of connected clients to stdout. 
				If the program is in client mode, it will get address
				info and get a socket and attempt to connect to a server.
				The client recieves input from the server and then closes
				when it writes the current date and time to stdout.

***********************************************************************/

/* Includes and definitions */
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define TIME_BYTES			19
#define BACKLOG 			5
#define MY_PORT_NUMBER 		49999
#define MY_PORT_NUMBER_S 	"49999"
#define BUFF_SIZE			1024

/*************************Server Function*********************************************/
void server(void) {
	int listenfd, numberRead;
	struct sockaddr_in servAddr;
	char buffer[BUFF_SIZE] = {0};
	time_t t;
	time(&t); //to get time

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd < 0) {
		perror("Error");
		exit(1);
	}
	if((setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int))) < 0) {
		perror("Error");
		exit(1);
	}
	

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET; 
	servAddr.sin_port = htons(MY_PORT_NUMBER); 
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(listenfd, (struct sockaddr*) &servAddr, sizeof(struct sockaddr_in)) < 0) {
		perror("Error");
		exit(1);
	}

	if(listen(listenfd, BACKLOG) < 0) { //Sets up a connection queue one level deep
		perror("Error");
		exit(1);
	}
	int connectfd, length = sizeof(struct sockaddr_in);
	struct sockaddr_in clientaddr;

	/*	Waits (blocks) until a connection is established by a client
		When that happens, a new descriptor for the connection is returned 
		If there is an error the result is < 0;
		The client’s address is written into the sockaddr structure
	*/
	char hostName[NI_MAXHOST]; 
	int hostEntry, connec_count = 0;
	while(1) {
		//accept clients
		connectfd = accept(listenfd,
							(struct sockaddr*) &clientaddr,
							(socklen_t*) &length);
		if(connectfd < 0) {
			perror("Error");
			exit(1);
		}

		connec_count++;
		if(!fork()) {		//child process
			//get hostname
			hostEntry = getnameinfo((struct sockaddr*)&clientaddr,  
									sizeof(clientaddr),
									hostName,
									sizeof(hostName),
									NULL,
									0,
									NI_NUMERICSERV);
			if(hostEntry != 0) {
				fprintf(stderr, "%s\n", gai_strerror(hostEntry));
				exit(1);
			} 

			printf("%s %d\n", hostName, connec_count); //print hostname out
			fflush(stdout);
			//make date/time string
			char* date = ctime(&t);
			date[TIME_BYTES-1] = '\n';
			if(write(connectfd, date, TIME_BYTES) < 0) {
				perror("Error");
				exit(1);
			}
			exit(0);
		} else { //parent waits
			wait(NULL);
		}
	}
}






/*************************Client Function*********************************************/
void client(char *addr) {
	int socketfd, err, numberRead;
	struct addrinfo hints, *actualData;
	memset(&hints, 0, sizeof(hints));
	char buffer[BUFF_SIZE] = {0};

	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family	  = AF_INET;

	err = getaddrinfo(addr, 
					  MY_PORT_NUMBER_S, 
					  &hints, 
					  &actualData);
	if(err != 0) {
		fprintf(stderr, "Error: %s\n", gai_strerror(err));
		exit(1);
	}
	socketfd = socket(actualData->ai_family, 
					  actualData->ai_socktype,
					  0);
	if(socketfd < 0) {
		perror("Error");
		exit(1);
	}

	if(connect(socketfd, actualData->ai_addr, actualData->ai_addrlen) < 0) {
		perror("Error");
		exit(1);
	}


	numberRead = read(socketfd, buffer, BUFF_SIZE);
	if(write(1, buffer, numberRead) < 0) {
		perror("Error");
		exit(1);
	}

	return;
}





/*************************Error Message*************************/
void printError(void) {
	printf("No parameter(s) given.\nExecute with:\tserver\n");
	printf("or:\t\tclient address\n");
	return;
}





/*****************************************************
			Main Function
 to decide whether to make server or to make a client
******************************************************/
int main(int argc, char const *argv[]){
	if(argc < 2) {			//no tokens given
		printError();
		return 1;
	} else if(argc == 2) { 
		if(strcmp(argv[1], "client") == 0) {	//if only 'client'
			printf("No address parameter given for client.\n");
			printf("Execute with: client address\n");
			return 1;
		} else if(strcmp(argv[1], "server") == 0) { //if: 'server'
			server();
			printf("server closed\n");
			return 0;
		} else printError();	//junk given
	} else if(argc > 2) { 
		if(strcmp(argv[1], "client") == 0) { //if: 'client' address
			char address[strlen(argv[2]) + 1];
			address[strlen(argv[2]) - 1] = '\0';
			strcpy(address, argv[2]);
			client(address);
			return 0;
		} else if(strcmp(argv[1], "server") == 0) { //if: 'server'
				server();
				return 0;
		} else printError();
	} else printError(); //if junk given

    return 0;
}
