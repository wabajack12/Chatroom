/* prog3_observer.c - code for example client program that uses TCP


*/



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/*------------------------------------------------------------------------
* Program: demo_client
*
* Purpose: allocate a socket, connect to a server, and print all output
*
* Syntax: ./demo_client server_address server_port
*
* server_address - name of a computer on which server is executing
* server_port    - protocol port number server is using
*
*------------------------------------------------------------------------
*/

void checkConnection(int status, int sd);

int main( int argc, char **argv) {
	struct hostent *ptrh; /* pointer to a host table entry */
	struct protoent *ptrp; /* pointer to a protocol table entry */
	struct sockaddr_in sad; /* structure to hold an IP address */
	int sd; /* socket descriptor */
	int port; /* protocol port number */
	char *host; /* pointer to host name */
	int n; /* number of characters read */

	memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
	sad.sin_family = AF_INET; /* set family to Internet */

	if( argc != 3 ) {
		fprintf(stderr,"Error: Wrong number of arguments\n");
		fprintf(stderr,"usage:\n");
		fprintf(stderr,"./client server_address server_port\n");
		exit(EXIT_FAILURE);
	}

	port = atoi(argv[2]); /* convert to binary */
	if (port > 0) /* test for legal value */
	sad.sin_port = htons((u_short)port);
	else {
		fprintf(stderr,"Error: bad port number %s\n",argv[2]);
		exit(EXIT_FAILURE);
	}

	host = argv[1]; /* if host argument specified */

	/* Convert host name to equivalent IP address and copy to sad. */
	ptrh = gethostbyname(host);
	if ( ptrh == NULL ) {
		fprintf(stderr,"Error: Invalid host: %s\n", host);
		exit(EXIT_FAILURE);
	}

	memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);

	/* Map TCP transport protocol name to protocol number. */
	if ( ((long int)(ptrp = getprotobyname("tcp"))) == 0) {
		fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
		exit(EXIT_FAILURE);
	}

	/* Create a socket. */
	sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
	if (sd < 0) {
		fprintf(stderr, "Error: Socket creation failed\n");
		exit(EXIT_FAILURE);
	}

	/* Connect the socket to the specified server. */
	if (connect(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
		fprintf(stderr,"connect failed\n");
		exit(EXIT_FAILURE);
	}

	char connected;
	char temp[100];
	char userName[10];
	char valid = 'T';
	recv(sd, &connected, 1, MSG_WAITALL);

	if(connected == 'Y'){
		int length = 0;
		while((length == 0 || length > 10) && valid == 'T' ){
			printf("Please enter the name you want to associate with: ");
			fgets(temp, sizeof(temp), stdin);
			length = strlen(temp) - 1;
			if(length > 10){
				printf("INVALID USER NAME\n");
			}

			for(int i = 0; i < length; i++){
				if((temp[i] < 48) || (temp[i] > 57 && temp[i] < 65) || (temp[i] > 90 && temp[i] <97) || (temp[i] > 122)){
					printf("INVALID USER NAME smorgy\n");
					length = 0;
				}
			}

			if(length < 10 && length > 0){
				uint8_t tempLength = strlen(temp) - 1;
				strcpy(userName, temp);
				send(sd, &tempLength, sizeof(uint8_t), 0);
				send(sd, &userName, tempLength, 0);
				recv(sd, &valid, 1, 0);
				if(valid == 'T'){
					length = 0;
				}
			}
		}
	}else{
		close(sd);
		printf("Server full\n");
		exit(1);
	}
	printf("Waiting on data\n");
	while(1){
		uint16_t length = 0;
		int status = recv(sd, &length, sizeof(uint16_t), MSG_WAITALL);
		if(status <= 0){
			printf("Server has disconnected you\n");
			printf("Goodbye\n");
			exit(1);
		}
		length = ntohs(length);
		char buf[length];
		recv(sd, buf, length, MSG_WAITALL);
		buf[length] = '\0';
		printf("%s\n", buf); // Only print newline if message doesnt have it
	}
	close(sd);

	exit(EXIT_SUCCESS);
}

void checkConnection(int status, int sd){
	if(status <= 0){
		close(sd);
		exit(1);
	}
}
