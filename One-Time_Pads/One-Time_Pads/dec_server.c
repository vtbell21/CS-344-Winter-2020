// Vincent Bell
// CS 344
// Assignment 5: One-Time Pads
// dec_server.c
// 3/2/21

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

// Error function used for reporting issues
void error(const char* msg) {
	perror(msg);
	exit(1);
}

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address,
	int portNumber) {

	// Clear out the address struct
	memset((char*)address, '\0', sizeof(*address));

	// The address should be network capable
	address->sin_family = AF_INET;
	// Store the port number
	address->sin_port = htons(portNumber);
	// Allow a client at any address to connect to this server
	address->sin_addr.s_addr = INADDR_ANY;
}

int main(int argc, char* argv[]) {

	int connectionSocket, charsRead;
	char* buffer = calloc((140000) + 1, sizeof(char));
	struct sockaddr_in serverAddress, clientAddress;
	socklen_t sizeOfClientInfo = sizeof(clientAddress);
	int bufferLength = 0;
	char* NewBuffer = calloc((140000)+1, sizeof(char));
	int NewBufferLength = 0;
	char* sent = calloc((140000) + 1, sizeof(char));

		// Check usage & args
		if (argc < 2) {
			fprintf(stderr, "USAGE: %s port\n", argv[0]);
			exit(1);
		}

		// Create the socket that will listen for connections
		int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (listenSocket < 0) {
			error("ERROR opening socket");
		}

		// Set up the address struct for the server socket
		setupAddressStruct(&serverAddress, atoi(argv[1]));

		// Associate the socket to the port
		if (bind(listenSocket,
			(struct sockaddr*)&serverAddress,
			sizeof(serverAddress)) < 0) {
			error("ERROR on binding");
		}

		// Start listening for connetions. Allow up to 5 connections to queue up
		listen(listenSocket, 5);

		// Accept a connection, blocking if one is not available until one connects
		while (1) {
			// Accept the connection request which creates a connection socket
			connectionSocket = accept(listenSocket,
				(struct sockaddr*)&clientAddress,
				&sizeOfClientInfo);
			if (connectionSocket < 0) {
				error("ERROR on accept");
			}

			// Get the message from the client and display it
			memset(buffer, '\0', 140000);
			// Read the client's message from the socket
			charsRead = recv(connectionSocket, buffer, 140000 - 1, 0);
			if (charsRead < 0) {
				error("ERROR reading from socket");
			}

			bufferLength = strlen(buffer);

			// Split the string when "!!" is found
			for (int i = 0; i < bufferLength; i++) {
				if ((buffer[i] == '!') && (buffer[i + 1] == '!')) {
					for (int j = 0; j < i; j++) {
						NewBuffer[j] = buffer[j];
						NewBufferLength++;
					}
				}
			}

			// Create the key string
			for (int k = 0; k < bufferLength - (bufferLength + 2); k++) {
				sent[k] = buffer[(bufferLength + 2) + k];
			}

			int textToInt = 0;
			int keyToInt = 0;
			int decryptToInt = 0;
			char* dec = calloc((140000) + 1, sizeof(char));

			// decryption
			for (int i = 0; i < NewBufferLength; i++) {
				// Convert each char to int 
				textToInt = abs(NewBuffer[i] - 65);
				keyToInt = abs(sent[i] - 65);

				decryptToInt = textToInt - keyToInt;

				if (decryptToInt < 0) {
					decryptToInt = decryptToInt + 27;
				}

				decryptToInt = decryptToInt % 27;		
				dec[i] = decryptToInt + 65 + 27;

				// Bug caused all 'A' to be '\' so this fixes it
				if (dec[i] == 92) {
					dec[i] = 65;
				}

				if (dec[i] == 87) {
					dec[i] = 80;
				}

				// If char is out of range then make it a space
				if (dec[i] < 65 || dec[i] > 90)
					dec[i] = ' ';	
			}

			// Send a Success message back to the client
			charsRead = send(connectionSocket, dec, strlen(dec), 0);
				if (charsRead < 0) {
					error("ERROR writing to socket");
				}
			
			// Close the connection socket for this client
			close(connectionSocket);
		}

		// Close the listening socket
		close(listenSocket);

	return 0;
}