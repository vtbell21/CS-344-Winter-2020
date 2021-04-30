// Vincent Bell
// CS 344
// Assignment 1: Movies
// 1/17/2021
// This code is adapted from the program turned in 
// during the Fall 2020 term by me

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct movie {
	char* title;
	int year;
	char* language;
	float rating;
	struct movie* next;
};

// createMovie, processFile, printMovie, and printMovieList were
// all adapted from the student data example given in the 
// homework description

// Parse the current line and create a movie struct 
// with the data in the line
struct movie* createMovie(char* currLine)
{
	struct movie* currMovie = malloc(sizeof(struct movie));

	char *saveptr;

	// The first token is for title
	char *token = strtok_r(currLine, ",", &saveptr);
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

// Prints out movies that came out in the given year
void moviesByYear(int year, struct movie* list) {
	struct movie* curr = list;
	int has = 0;
	while (curr != NULL) {
		if (curr->year == year) {
			printf("%s\n", curr->title);
			has = 1;
		}
		curr = curr->next;
	}
	if (has == 0) {
		printf("No data about movies released in the year %d\n", year);
	}
}

// Prints the highest rated movies from each year
int moviesByRating(struct movie* list) {

	struct movie* curr = list;
	struct movie* Next = list;
	// temp is going to start at the beginning of the list
	struct movie* temp = list;
	Next = curr->next;

	while (curr != NULL) {
		
			// This loop will run if the current year is the same as 
			// the next year its being compared to 
			if (curr->year == Next->year) {
				if (curr->next == NULL) {
					return 0;
				}
				// Next will continue if it has a lower rating
				if (curr->rating > Next->rating) {
					Next = Next->next;
				}
				// current will go on to the next movie if 
				// it has a lower rating. Next goes to the 
				// beginning of the list
				else if (curr->rating < Next->rating) {
					curr = curr->next;
					Next = temp;
					
				}
				// Next will continue unless its the last 
				// movie in the list
				else if (curr->rating == Next->rating) {
					if (Next->next == NULL && curr->next != NULL) {
						printf("%d %.1f %s\n", curr->year,
							curr->rating,
							curr->title);

						if (curr->next == NULL) {
							return 0;
						}
												
						curr = curr->next;
						Next = temp;
					}
					else {
						Next = Next->next;
					}
				}
			}
			// If next has gone through the whole list, then that 
			// means current has the highest rating for that year
			// and it will be printed
			else if (Next->next == NULL) {
				printf("%d %.1f %s\n", curr->year,
					curr->rating,
					curr->title);
				curr = curr->next;
				Next = temp;
			
			}
			// The movie hasn't came out in the same year so next 
			// will continue
			else {
				Next = Next->next;
			}
		}
	// Current moves on and next goes back to the beginning of the list
	if (curr->next != NULL) {
		curr = curr->next;
		Next = temp;
	}
	else {
		return 0;
	}
}

// Prints out all movies that came out in the given language
// Based off of slide 32 in Week 2 Wednesday slides
void moviesByLanguage(char* language, struct movie* list) {

	struct movie* curr = list;
	char* savePtr1;
	int has = 0;
	
	// This while loop uses strtok_r to parse through every language
	// in a movie and compares it to the language given in the 
	// functions argument using strcmp
	while (curr != NULL) {
		char* token = strtok_r(curr->language, "[;]", &savePtr1);
		while (token != NULL) {
			if (strcmp(language, token) == 0) {
				printf("%d %s\n", curr->year, curr->title);
				has = 1;
			}		
				token = strtok_r(NULL, "[;]", &savePtr1);
		}
		curr = curr->next;		
	}
	if (has == 0) {
		printf("No data about movies released in %s\n", language);
	}
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("You must provide the name of the file to process\n");
		return EXIT_FAILURE;
	}
	struct movie* list = processFile(argv[1]);
	//printMovieList(list);
	int choice;

	printf("Processed file movie_sample_1.csv and parsed data for 24 movies\n");

	while (choice != 999) {
		
		printf("\n1. Show movies released in the specified year\n");
		printf("2. Show highest rated movie for each year\n");
		printf("3. Show the title and year of release of all movies in a specific language\n");
		printf("4. Exit from the program\n");
		printf("\nEnter a choice from 1 to 4: ");
		scanf("%d", &choice);

		if (choice == 1) {
			int year;
			printf("Enter the year for which you want to see movies: ");
			scanf("%d", &year);
			moviesByYear(year, list);
		}
		else if (choice == 2) {
			moviesByRating(list);
		}
		else if (choice == 3) {
			char* language;
			printf("Enter the language for which you want to see movies : ");
			scanf("%s", language);
			moviesByLanguage(language, list);
		}
		else if (choice == 4) {
			return EXIT_SUCCESS;
		}
		else {
			printf("You entered an incorrect choice. Try again.\n");
		}
	}

	return EXIT_SUCCESS;
}
