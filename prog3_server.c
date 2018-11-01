
/* demo_server.c - code for example server program that uses TCP */
//Owen Sheets
//CS367
//Assignment3

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>
#include "trie.h"

typedef struct clientStruct {
	int sd1; // Particpant Socket
	int sd2; // Observer socket
	int flag;
	char* name;
} client;

void connectP(int sd, struct sockaddr_in *cad); // functions to connect people
void connectO(int sd, struct sockaddr_in *cad2);

int usernamePart(int sdTemp); // Functions to get username
void usernameObs(int sdTemp);

void checkConnectionPart(int status, client* part); // Functions to check connections
void checkConnectionObs(int status, int sd);

void broadcastMsg(char* message, uint16_t length); // Functions to send msgs
void sendMsg(char* message, uint16_t length, int sd);
void broadcast(char* name, uint16_t length);

client* createClient();

#define QLEN 6 /* size of request queue */
int visits = 0; /* counts client connections */
int alen; /* length of address */

struct TrieNode* usernames;
int numPart = 0;
int numObs = 0;
int observers[255];
int participants[255];
client* listOfClients[255];
int max = 0;
fd_set sockets;

char n = 'N'; // These are lazy ways to generate messages
char y = 'Y';
char t = 'T';
char userMsg[14] = ">           : ";
char userJoin[26] = "User            has joined";
char userLeft[24] = "User            has left";
char obsJoin[25] = "A new observer has joined";
/*------------------------------------------------------------------------
* Program: demo_server
*
* Purpose: allocate a socket and then repeatedly execute the following:
* (1) wait for the next connection from a client
* (2) send a short message to the client
* (3) close the connection
* (4) go back to step (1)
*
* Syntax: ./demo_server port
*
* port - protocol port number to use
*
*------------------------------------------------------------------------
*/


void checkConnectionPart(int status, client* part){
	if(status <= 0){
		close(part->sd1);
		close(part->sd2);
		for(int i = 0; i < numPart; i++){ // remove sd from participants
			if(part->sd1 == participants[i]){
				participants[i] = 0;
			}
		}
		for(int i = 0; i < numObs; i++){ // remove sd from observers
			if(part->sd2 == observers[i]){
				observers[i] = 0;
			}
		}
		numPart--;
		uint16_t length = strlen(part->name);
		uint16_t newLength = 26;
		char* message;
		message = strdup(userLeft);
		int index = 0;
		for(int i = 5; i < 5 + length; i++){
			message[i] = part->name[index];
			index++;
		}
		broadcast(message, newLength);
		part = NULL;
	}
}

void checkConnectionObs(int status, int sd){
	if(status <= 0){
		close(sd);
		for(int i = 0; i < numObs; i++){ // remove sd from observers
			if(observers[i] == sd){
				observers[i] = 0;
			}
		}
	}
}

client* createClient(){
	client* temp = (client*)malloc(sizeof(client));
	temp->sd1 = 0;
	temp->sd2 = 0;
	temp->flag = 0;
	char* name = NULL;
}

int main(int argc, char **argv) {
	struct protoent *ptrp; /* pointer to a protocol table entry */
	struct sockaddr_in sad; /* structure to hold server's address */
	struct sockaddr_in sad2; /* structure to hold server's address */
	struct sockaddr_in cad; /* structure to hold client's address */
	struct sockaddr_in cad2; /* structure to hold client's address */
	int sd1, sd2;
	int port; /* protocol port number */
	int port2;
	int optval = 1; /* boolean value when we set socket option */

	usernames = getNode();

	for(int i = 0; i < 255; i++){
		observers[i] = 0;
		participants[i] = 0;
		listOfClients[i] = NULL;
	}

	if( argc != 3 ) {
		fprintf(stderr,"Error: Wrong number of arguments\n");
		fprintf(stderr,"usage:\n");
		fprintf(stderr,"./server server_port word\n");
		exit(EXIT_FAILURE);
	}

	memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
	sad.sin_family = AF_INET; /* set family to Internet */
	sad.sin_addr.s_addr = INADDR_ANY; /* set the local IP address */

	port = atoi(argv[1]); /* convert argument to binary */

	if (port > 0) { /* test for illegal value */
		sad.sin_port = htons((u_short)port);
	} else { /* print error message and exit */
		fprintf(stderr,"Error: Bad port number %s\n",argv[1]);
		exit(EXIT_FAILURE);
	}

	/* Map TCP transport protocol name to protocol number */
	if ( ((long int)(ptrp = getprotobyname("tcp"))) == 0) {
		fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
		exit(EXIT_FAILURE);
	}

	/* Create a socket */
	sd1 = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
	if (sd1 < 0) {
		fprintf(stderr, "Error: Socket creation failed\n");
		exit(EXIT_FAILURE);
	}

	/* Allow reuse of port - avoid "Bind failed" issues */
	if( setsockopt(sd1, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0 ) {
		fprintf(stderr, "Error Setting socket option failed\n");
		exit(EXIT_FAILURE);
	}

	/* Bind a local address to the socket */
	if (bind(sd1, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
		fprintf(stderr,"Error: Bind failed\n");
		exit(EXIT_FAILURE);
	}

	/* Specify size of request queue */
	if (listen(sd1, QLEN) < 0) {
		fprintf(stderr,"Error: Listen failed\n");
		exit(EXIT_FAILURE);
	}
	//******************************************************************
	//******************************************************************
	memset((char *)&sad2,0,sizeof(sad2)); /* clear sockaddr structure */
	sad2.sin_family = AF_INET; /* set family to Internet */
	sad2.sin_addr.s_addr = INADDR_ANY; /* set the local IP address */

	port2 = atoi(argv[2]); /* convert argument to binary */

	if (port2 > 0) { /* test for illegal value */
		sad2.sin_port = htons((u_short)port2);
	} else { /* print error message and exit */
		fprintf(stderr,"Error: Bad port number %s\n",argv[2]);
		exit(EXIT_FAILURE);
	}

	/* Map TCP transport protocol name to protocol number */
	if ( ((long int)(ptrp = getprotobyname("tcp"))) == 0) {
		fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
		exit(EXIT_FAILURE);
	}

	/* Create a socket */
	sd2 = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
	if (sd2 < 0) {
		fprintf(stderr, "Error: Socket creation failed\n");
		exit(EXIT_FAILURE);
	}

	/* Allow reuse of port - avoid "Bind failed" issues */
	if( setsockopt(sd2, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0 ) {
		fprintf(stderr, "Error Setting socket option failed\n");
		exit(EXIT_FAILURE);
	}

	/* Bind a local address to the socket */
	if (bind(sd2, (struct sockaddr *)&sad2, sizeof(sad2)) < 0) {
		fprintf(stderr,"Error: Bind failed\n");
		exit(EXIT_FAILURE);
	}

	/* Specify size of request queue */
	if (listen(sd2, QLEN) < 0) {
		fprintf(stderr,"Error: Listen failed\n");
		exit(EXIT_FAILURE);
	}

	/* Main server loop - accept and handle requests */
	max = sd2;
	int status = 0;
	while (1) {
		status = 0;
		FD_ZERO(&sockets); // SD set maitenence
		FD_SET(sd1, &sockets);
		FD_SET(sd2, &sockets);
		for(int i = 0; i < numPart; i++){
			FD_SET(participants[i], &sockets);
			if(participants[i] > max){
				max = participants[i];
			}
		}
		for(int i = 0; i < numObs; i++){
			FD_SET(observers[i], &sockets);
			if(observers[i] > max){
				max = observers[i];
			}
		}
		// RE ADD EVERY SOCKET AT BEGGINING OF LOOP
		status = select(max + 1, &sockets, NULL, NULL, NULL);
		if(status==-1)
		{
			printf("ERROR\n");
			strerror(errno);
		}
		for(int i = 0; i < max + 1; i++){
			if(FD_ISSET(i, &sockets)){
				if(i == sd1){ // Participant is trying to connect
					connectP(sd1, &cad);
				}else if(i == sd2){ // Observer trying to connect
					connectO(sd2, &cad2);
				}else{
					int flag = 0; // Flag to see if sender is trying to get username
					for(int k = 0; k < numObs; k++){ // Observers never send data unless it
						if(i == observers[k]){ // is trying to get username
							usernameObs(i);
							flag = 1;
						}
					}
					if (flag == 0) {
						for(int j = 0; j < numPart; j++){ // Go through participants and see if one is trying to get username
							if(listOfClients[j]->flag == 0 && i == listOfClients[j]->sd1){
								printf("Negotiating username for participant\n");
								int result = usernamePart(i);
								if(result == 1){
									listOfClients[j]->flag = 1;
								}
								flag = 1;
							}
						}
					}
					if(flag == 0){ // Someone is trying to send a msg
						char buf[1000];
						uint16_t msgLength;
						client* cur = NULL;
						for(int j = 0; j < numPart; j++){ // Find which client wants to send msg
							if(listOfClients[j]->sd1 == i){
								cur = listOfClients[j];
								break;
							}
						}
					  checkConnectionPart(recv(i, &msgLength, sizeof(uint16_t), 0), cur);
						recv(i, buf, msgLength, 0);
						int private = 0;
						if(buf[0] == '@'){
							private = 1;
						}
						char message[msgLength + 14];
						strcpy(message, userMsg);
						int index = 0;
						char* name2 = cur->name;
						for(int i = strlen(userMsg) - strlen(cur->name) -2; i < 12; i++){
							message[i] = name2[index];
							index++;
						}
						index = 0;
						for(int i = 14; i < ntohs(msgLength) + 14; i++){
							message[i] = buf[index];
							index++;
						}
						if(private == 1){
							char dest[10];
							message[0] = '-';
							for(int i = 1; i < 11; i++){
								if(!(isspace(buf[i]))){
									dest[i - 1] = buf[i];
								}else{
									dest[i - 1] = '\0';
									break;
								}
							}
							int sdDest = 0;
							for(int i = 0; i < numPart; i++){
								if(strcmp(listOfClients[i]->name, dest) == 0){
									sdDest = listOfClients[i]->sd2;
								}
							}
							sendMsg(message, msgLength, sdDest);
						}else{
							broadcastMsg(message, msgLength);
						}
					}
				}
			}
		}
	}
}

void broadcast(char* name, uint16_t length){ // Function to broadcast when a participant has joined/left
	length = htons(length);

	for(int i =0; i < numObs; i++){
		if(observers[i] != 0){
			send(observers[i], &length, sizeof(uint16_t), 0);
			send(observers[i], name, ntohs(length), 0);
		}
	}
}

void broadcastMsg(char* message, uint16_t length){ // Function to broadcast msg to all observers
	length = htons(length);
	for(int i = 0; i < numObs; i++){
		if(observers[i] != 0){
			send(observers[i], &length, sizeof(uint16_t), 0);
			send(observers[i], message, ntohs(length), 0);
		}
	}
}

void sendMsg(char* message, uint16_t length, int sd){ //Function to send private messages
	length = htons(length);
	send(sd, &length, sizeof(uint16_t), 0);
	send(sd, message, ntohs(length), 0);
}

void connectP(int sd, struct sockaddr_in *cad){ // Function to connect participants
	alen = sizeof(cad);
	int sdTemp = 0;
	if ( (sdTemp=accept(sd, (struct sockaddr *)&cad, &alen)) < 0) {
		fprintf(stderr, "Error: Accept failed\n");
		exit(EXIT_FAILURE);
	}
	if(numPart >= 255){
		send(sdTemp, &n, 1, 0);
		close(sdTemp);
		return;
	}
	int i = 0;
	while(participants[i] != 0){
		i++;
	}
	participants[i] = sdTemp;
	numPart++;
	client* new = createClient();
	new->sd1 = sdTemp;
	checkConnectionPart(send(sdTemp, &y, 1, 0), new);
	int index = 0;
	while(listOfClients[index] != NULL){
		index++;
	}
	listOfClients[index] = new;
	printf("Connected Participant %d\n", listOfClients[index]->sd1);
}

void connectO(int sd, struct sockaddr_in *cad2){ // Function to connect observers
	alen = sizeof(cad2);
	int sdTemp = 0;
	if ( (sdTemp=accept(sd, (struct sockaddr *)&cad2, &alen)) < 0) {
		fprintf(stderr, "Error: Accept failed\n");
		exit(EXIT_FAILURE);
	}
	if(numObs >= 255){
		send(sdTemp, &n, 1, 0);
		close(sdTemp);
		return;
	}
	int i = 0;
	while(observers[i] != 0){
		i++;
	}
	observers[i] = sdTemp;
	numObs++;
	checkConnectionObs(send(sdTemp, &y, 1, 0), sdTemp);
	printf("Observer Connected %d\n", sdTemp);
}

int usernamePart(int sdTemp){ // Function to get username of participants
	uint8_t length = 0;
	client* cur = NULL;
	char* nameTemp;
	recv(sdTemp, &length, sizeof(uint8_t), 0);
	char name[length];
	nameTemp = name;
	recv(sdTemp, &name, length, 0);
	name[length] = '\0';
	if(search(usernames, name)){
		send(sdTemp, &t ,1 ,0);
		return 0;
	}else{
		for(int i = 0; i < numPart; i++){
			if(listOfClients[i]->sd1 == sdTemp){
				listOfClients[i]->sd1 = sdTemp;
				listOfClients[i]->name = strdup(name);
				cur = listOfClients[i];
				break;
			}
		}
		insert(usernames, name);
		send(sdTemp, &y, 1, 0);
	}
	uint16_t newLength = 26;

	char* message;
	message = strdup(userJoin);
	int index = 0;
	for(int i = 5; i < 5 + length; i++){
		message[i] = cur->name[index];
		index++;
	}
	broadcast(message, newLength);
	return 1;
}

void usernameObs(int sdTemp){ // Function to get observer username
	uint8_t length = 0;
	length = 0;
	printf("Negotating observer name\n");       // IF THIS IS TAKEN OUT THE WHOLE THING BREAKS
	recv(sdTemp, &length, sizeof(uint8_t), MSG_WAITALL);
	char name[length + 1];
	checkConnectionObs(recv(sdTemp, &name, length, MSG_WAITALL), sdTemp);
	name[length + 1] = '\0';

	if(search(usernames, name)){
		send(sdTemp, &y, 1, 0);
		for(int i = 0; i < numObs; i++){
			if(strcmp(listOfClients[i]->name, name) == 0 ){
				listOfClients[i]->sd2 = sdTemp;
				length = 25;
				broadcast(obsJoin, length);
				break;
			}
		}
	}else{
		send(sdTemp, &t ,1 ,0);
		length = 0;
		return;
	}
}
