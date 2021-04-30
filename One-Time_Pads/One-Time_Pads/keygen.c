// Vincent Bell
// CS 344
// Assignment 5: One-Time Pads
// keygen.c
// 3/2/21

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char* argv[]) {

	// Print error message if keygen is not provided
	if (argc < 2) {
		printf("Did not enter the length for keygen\n");
		return 0;
	}

	// Set starting point for random integers
	srand(time(0));

	int length = atoi(argv[1]);
	char key[length + 1];
	int charVal = 0;
	char numToChar;

	for (int i = 0; i < length; i++) {
		// charVal is a random ascii value in the range of the alphabet
		charVal = rand() % 27 + 65;
		// If CharVal is out of ascii range of alphabet, Then set it to a space
		if (charVal == 91) {
			charVal = 32;
		}
		// Setting key[i] = charVal will make a char equal to the ascii value of the char
		key[i] = charVal;
	}
	// Make the end of the key a new line
	key[length] = '\n';

	printf("%s", key);
	fflush(stdout);

	return 0;
}