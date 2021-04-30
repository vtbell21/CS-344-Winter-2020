// Vincent Bell
// CS 344
// Assignment 2: Files & Directories
// 1/25/2021
// This assignment is adapted from the Fall 2020
// assignment that I completed

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

struct movie {
	char* title;
	int year;
	char* language;
	float rating;
	struct movie* next;
};

// createMovie, processFile, printMovie, and printMovieList were
// all adapted from the student data example given in the 
// homework 1 description

// Parse the current line and create a movie struct 
// with the data in the line
struct movie* createMovie(char* currLine)
{
	struct movie* currMovie = malloc(sizeof(struct movie));

	char* saveptr;

	// The first token is for title
	char* token = strtok_r(currLine, ",", &saveptr);
	currMovie->title = calloc(strlen(token) + 1, sizeof(char));
	strcpy(currMovie->title, token);

	// The next token is for year
	int year_token = atoi(strtok_r(NULL, ",", &saveptr));
	currMovie->year = year_token;

	// The next token is for language
	token = strtok_r(NULL, ",", &saveptr);
	currMovie->language = calloc(strlen(token) + 1, sizeof(char));
	strcpy(currMovie->language, token);

	// The next token is for rating
	float ratingtoken = strtof(strtok_r(NULL, "\n", &saveptr), NULL);
	currMovie->rating = ratingtoken;

	currMovie->next = NULL;

	return currMovie;
}

struct movie* processFile(char* filePath)
{
	FILE* movieFile = fopen(filePath, "r");

	char* currLine = NULL;
	size_t len = 0;
	ssize_t nread;
	char* token;

	// The head of the linked list
	struct movie* head = NULL;
	// The tail of the linked list
	struct movie* tail = NULL;

	// getLine will ignore the first line of the file
	getline(&currLine, &len, movieFile);

	// Read the file line by line
	while ((nread = getline(&currLine, &len, movieFile)) != -1)
	{
		// Get a new movie node corresponding to the current line
		struct movie* newNode = createMovie(currLine);

		// Is this the first node in the linked list?
		if (head == NULL)
		{
			// This is the first node in the linked link
			// Set the head and the tail to this node
			head = newNode;
			tail = newNode;
		}
		else
		{
			// This is not the first node.
			// Add this node to the list and advance the tail
			tail->next = newNode;
			tail = newNode;
		}
	}
	free(currLine);
	fclose(movieFile);
	return head;
}

/*
* Print data for the given movie
*/
void printMovie(struct movie* aMovie) {
	printf("%s, %d %s, %f\n", aMovie->title,
		aMovie->year,
		aMovie->language,
		aMovie->rating);
}

/*
* Print the linked list of movies
*/
void printMovieList(struct movie* list)
{
	while (list != NULL)
	{
		printMovie(list);
		list = list->next;
	}
}

// Finds smallest file
char* smallestFile() {
	DIR* currDir = opendir(".");
	struct dirent* aFile;
	int size = 9999;
	char* prefix = "movies_";
	char* exten = ".csv";
	struct stat dirStat;
	char* FileName = NULL;

	// Compares the sizes of each file and returns the file name
	// of the smallest file
	while ((aFile = readdir(currDir)) != NULL) {
		stat(aFile->d_name, &dirStat);

		int len = strlen(aFile->d_name);
		char* FileExten = &aFile->d_name[len - strlen(exten)];

		if (strncmp(prefix, aFile->d_name, strlen(prefix)) == 0) {

			if (strcmp(exten, FileExten) == 0) {
				// compares size of file to size of previous file.
				// size is first set to 9999 so the first file will
				// be chosen
				if (dirStat.st_size < size) {
					size = dirStat.st_size;
					FileName = aFile->d_name;
				}
			}
		}
	}

	return FileName;
}

// Finds largest file
char* largestFile() {
	DIR* currDir = opendir(".");
	struct dirent* aFile;
	int size = 0;
	char* prefix = "movies_";
	char* exten = ".csv";
	struct stat dirStat;
	char* FileName = NULL;

	// Compares the sizes of each file and returns the file name
	// of the largest file
	while ((aFile = readdir(currDir)) != NULL) {
		stat(aFile->d_name, &dirStat);

		int len = strlen(aFile->d_name);
		char* FileExten = &aFile->d_name[len - strlen(exten)];

		if (strncmp(prefix, aFile->d_name, strlen(prefix)) == 0) {

			if (strcmp(exten, FileExten) == 0) {
				// compares size of file to size of previous file.
				// size is first set to 0 so the first file will
				// be chosen
				if (dirStat.st_size > size) {
					size = dirStat.st_size;
					FileName = aFile->d_name;
				}
			}
		}
	}

	return FileName;
}

// Creates Directory
void createDir(struct movie* list) {
	// make directory name of bellv.movies.random
	srand(time(NULL));
	int r = 1 + (rand() % 99999);
	char randomString[6];
	sprintf(randomString, "%d", r);
	char dirName[30] = "bellv.movies.";
	strcat(dirName, randomString);

	printf("Created directory with name %s", dirName);
	printf("\n");

	struct movie* curr = list;

	// make the directory with permissions set to drwxr-x---
	mkdir(dirName, 0750);

	DIR* dir = opendir(dirName);
	char newFile[32];
	int file_descriptor;
	int wr;
	int year = 0;

	// Parses through the data in the csv file and creates a file
	// for each year a movie was released within the new directory
	while (curr != NULL) {
		sprintf(newFile, "%s/%d.txt", dirName, curr->year);
		// Sets file permission to rw-r-----
		file_descriptor = open(newFile, O_CREAT | O_RDWR | O_APPEND, 0640);
		// Writes movie title into YYYY.txt if the years match
		if (curr->year == year) {
			write(file_descriptor, "\n", strlen("\n") + 1);
			write(file_descriptor, curr->title, strlen(curr->title) + 1);
		}
		if (curr->year != year) {
			write(file_descriptor, curr->title, strlen(list->title)+1);
			write(file_descriptor, "\n", strlen("\n"));
		}
		close(file_descriptor);
		year = curr->year;
		curr = curr->next;
	}
}

// Asks the user to pick an option on how they want to
// process the files and calls the appropriate functions
// to do so
void runFileProcess() {
	printf("Which file do you want to process?\n");
	printf("Enter 1 to pick the largest file\n");
	printf("Enter 2 to pick the smallest file\n");
	printf("Enter 3 to specify the name of a file\n");
	printf("\n");

	int choice;

	printf("Enter a choice from 1 to 3: ");
	scanf("%d", &choice);
	
	if (choice == 1) {
		char* file = largestFile();
		struct movie* list = processFile(file);
		printf("Now processing the chosen file named %s", file);
		printf("\n");
		createDir(list);
	}
	else if (choice == 2) {
		char* file = smallestFile();
		struct movie* list = processFile(file);
		printf("Now processing the chosen file named %s", file);
		printf("\n");
		createDir(list);
	}
	else if (choice == 3) {
		// The file name can be passed directly into createDir()
		char inputFile[20];
		printf("Enter the complete file name: ");	
		scanf("%s", inputFile);
		//printf("\n");

		// Checks if the file exists
		if( access( inputFile, F_OK ) == 0 ) {
		struct movie* list = processFile(inputFile);
		printf("Now processing the chosen file named %s", inputFile);
		printf("\n");
		createDir(list);
		}
		else{
			printf("The file %s was not found. Try again", inputFile);
			printf("\n");
			printf("\n");
			runFileProcess();
		}	

	}
	else {
		printf("You entered an incorrect choice. Try again.");
	}
}

int main()
{
	int choice;

	while (choice != 999) {
		printf("\n1. Select file to process\n");
		printf("2. Exit the program\n");
		printf("\nEnter a choice 1 or 2: ");
		scanf("%d", &choice);
		printf("\n");

		if (choice == 1) {
			runFileProcess();
		}
		else if (choice == 2) {
			return EXIT_SUCCESS;
		}
		else {
			printf("You entered an incorrect choice. Try again.");
		}
	}

	return EXIT_SUCCESS;
}

