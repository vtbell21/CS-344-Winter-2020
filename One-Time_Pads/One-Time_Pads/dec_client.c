// Vincent Bell
// CS 344
// Assignment 5: One-Time Pads
// dec_client.c
// 3/2/21

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()
#include <sys/stat.h>
#include <sys/wait.h>

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/

// Error function used for reporting issues
void error(const char* msg) {
	perror(msg);
	exit(0);
}

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address,
	int portNumber,
	char* hostname) {

	// Clear out the address struct
	memset((char*)address, '\0', sizeof(*address));

	// The address should be network capable
	address->sin_family = AF_INET;
	// Store the port number
	address->sin_port = htons(portNumber);

	// Get the DNS entry for this host name
	struct hostent* hostInfo = gethostbyname(hostname);
	if (hostInfo == NULL) {
		fprintf(stderr, "CLIENT: ERROR, no such host\n");
		exit(0);
	}
	// Copy the first IP address from the DNS entry to sin_addr.s_addr
	memcpy((char*)&address->sin_addr.s_addr,
		hostInfo->h_addr_list[0],
		hostInfo->h_length);
}

int main(int argc, char* argv[]) {
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	char* buffer = calloc((140000) + 1, sizeof(char));
	char* key = calloc((140000) + 1, sizeof(char));
	char* master = calloc((140000) + 1, sizeof(char));
	char* received = calloc((140000) + 1, sizeof(char));
	memset(received, '\0', 140000);
	char* Allreceived = calloc((140000) + 1, sizeof(char));
	size_t length = 0;
	int bufferLength = 0;
	int keyLength = 0;

	// Check usage & args
	if (argc < 3) {
		fprintf(stderr, "Please enter text, key, and port");
		exit(1);
	}

	// Get information from text file
	FILE* textFile = fopen(argv[1], "r");
	getline(&buffer, &length, textFile);
	bufferLength = strlen(buffer);

	// Get information from the key file
	length = 0;
	FILE* keyFile = fopen(argv[2], "r");
	getline(&key, &length, keyFile);
	keyLength = strlen(key);

	if (keyLength < bufferLength) {
		fprintf(stderr, "The Key file is too small for this text file\n");
		exit(1);
	}

	// Creates master string with the chars '!!' in between to split text and key
	strcpy(master, buffer);

	// Create the buffer 
	master[bufferLength - 1] = '!';
	master[bufferLength] = '!';

	// Add the keyBuffer to the master 
	strcat(master, key);

	// Create a socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFD < 0) {
		error("CLIENT: ERROR opening socket");
	}

	// Set up the server address struct
	setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
		error("CLIENT: ERROR connecting");
	}

	// Send message to server
	// Write to the server
	charsWritten = send(socketFD, master, strlen(master), 0);
	if (charsWritten < 0) {
		error("CLIENT: ERROR writing to socket");
	}
	if (charsWritten < strlen(master)) {
		printf("CLIENT: WARNING: Not all data written to socket!\n");
	}

	// Get return message from server
   // Read data from the socket, leaving \0 at end
	while (strlen(Allreceived) <= bufferLength - 1) {
		charsRead = recv(socketFD, received, 1, 0);
		strcat(Allreceived, received);
		if (charsRead < 0) {
			error("CLIENT: ERROR reading from socket");
		}
	}
	// Prints full message from the Server
	printf("%s", Allreceived);
	printf("\n");
	fflush(stdout);

	// Close the socket
	close(socketFD);

	return 0;
}