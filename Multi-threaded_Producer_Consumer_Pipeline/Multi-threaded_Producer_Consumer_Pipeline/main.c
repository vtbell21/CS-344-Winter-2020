// Vincent Bell
// CS 344
// Assignment 4: Multi-threaded Producer Consumer Pipeline
// 2/21/21

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#define INPUT_SIZE 1000
#define OUTPUT_SIZE 80
#define NUM_THREADS 4

// Code is adapted from https://repl.it/@cs344/65prodconspipelinec

// Buffer 1, shared resource between input thread and square-root thread
char* buffer_1[INPUT_SIZE];
// Number of items in the buffer
int count_1 = 0;
// Index where the input thread will put the next item
int prod_idx_1 = 0;
// Index where the square-root thread will pick up the next item
int con_idx_1 = 0;
// Initialize the mutex for buffer 1
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 1
pthread_cond_t full_1 = PTHREAD_COND_INITIALIZER;

// Buffer 2, shared resource between square root thread and output thread
char* buffer_2[INPUT_SIZE];
// Number of items in the buffer
int count_2 = 0;
// Index where the square-root thread will put the next item
int prod_idx_2 = 0;
// Index where the output thread will pick up the next item
int con_idx_2 = 0;
// Initialize the mutex for buffer 2
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 2
pthread_cond_t full_2 = PTHREAD_COND_INITIALIZER;

// Buffer 2, shared resource between square root thread and output thread
char* buffer_3[INPUT_SIZE];
// Number of items in the buffer
int count_3 = 0;
// Index where the square-root thread will put the next item
int prod_idx_3 = 0;
// Index where the output thread will pick up the next item
int con_idx_3 = 0;
// Initialize the mutex for buffer 2
pthread_mutex_t mutex_3 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 2
pthread_cond_t full_3 = PTHREAD_COND_INITIALIZER;

void put_buff_1(char* item);
char* get_buff_1();
void put_buff_2(char* item);
char* get_buff_2();
void put_buff_3(char* item);
char* get_buff_3();

char* get_user_input();

void *input_thread();
void *line_seperator_thread();
void *plus_sign_thread();
void *output_thread();

// Put an item in buffer_1
void put_buff_1(char* item){
	// Lock the mutex before putting the item in the buffer
	pthread_mutex_lock(&mutex_1);
	// Put the item in the buffer
	buffer_1[prod_idx_1] = item;
	// Increment the index where the next item will be put.
	prod_idx_1 = prod_idx_1 + 1;
	count_1++;
	// Signal to the consumer that the buffer is no longer empty
	pthread_cond_signal(&full_1);
	// Unlock the mutex
	pthread_mutex_unlock(&mutex_1);
}

// Get the next item from buffer 1
char* get_buff_1() {
	// Lock the mutex before checking if the buffer has data
	pthread_mutex_lock(&mutex_1);
	while (count_1 == 0)
		// Buffer is empty. Wait for the producer to signal that the buffer has data
		pthread_cond_wait(&full_1, &mutex_1);
	char* item = buffer_1[con_idx_1];
	// Increment the index from which the item will be picked up
	con_idx_1 = con_idx_1 + 1;
	count_1--;
	// Unlock the mutex
	pthread_mutex_unlock(&mutex_1);
	// Return the item
	return item;
}

// Put an item in buffer_2
void put_buff_2(char* item) {
	// Lock the mutex before putting the item in the buffer
	pthread_mutex_lock(&mutex_2);
	// Put the item in the buffer
	buffer_2[prod_idx_2] = item;
	// Increment the index where the next item will be put.
	prod_idx_2 = prod_idx_2 + 1;
	count_2++;
	// Signal to the consumer that the buffer is no longer empty
	pthread_cond_signal(&full_2);
	// Unlock the mutex
	pthread_mutex_unlock(&mutex_2);
}

// Get the next item from buffer 2
char* get_buff_2() {
	// Lock the mutex before checking if the buffer has data
	pthread_mutex_lock(&mutex_2);
	while (count_2 == 0)
		// Buffer is empty. Wait for the producer to signal that the buffer has data
		pthread_cond_wait(&full_2, &mutex_2);
	char* item = buffer_2[con_idx_2];
	// Increment the index from which the item will be picked up
	con_idx_2 = con_idx_2 + 1;
	count_2--;
	// Unlock the mutex
	pthread_mutex_unlock(&mutex_2);
	// Return the item
	return item;
}

// Put an item in buffer_3
void put_buff_3(char* item) {
	// Lock the mutex before putting the item in the buffer
	pthread_mutex_lock(&mutex_3);
	// Put the item in the buffer
	buffer_3[prod_idx_3] = item;
	// Increment the index where the next item will be put.
	prod_idx_3 = prod_idx_3 + 1;
	count_3++;
	// Signal to the consumer that the buffer is no longer empty
	pthread_cond_signal(&full_3);
	// Unlock the mutex
	pthread_mutex_unlock(&mutex_3);
}

// Get the next item from buffer 3
char* get_buff_3() {
	// Lock the mutex before checking if the buffer has data
	pthread_mutex_lock(&mutex_3);
	while (count_3 == 0)
		// Buffer is empty. Wait for the producer to signal that the buffer has data
		pthread_cond_wait(&full_3, &mutex_3);
	char* item = buffer_3[con_idx_3];
	// Increment the index from which the item will be picked up
	con_idx_3 = con_idx_3 + 1;
	count_3--;
	// Unlock the mutex
	pthread_mutex_unlock(&mutex_3);
	// Return the item
	return item;
}

char* get_user_input() {
	
	char* input = NULL;
	size_t length = INPUT_SIZE;
	ssize_t result;

	result = getline(&input, &length, stdin);
	return input;
}

// Reads in lines of characters from standard input
void* input_thread() {

	bool input_stop = false;
	char* line = NULL;

	while (!input_stop) {
		line = get_user_input();

		if (!strcmp(line, "STOP\n")) {
			input_stop = true;
		}

		put_buff_1(line);
	}

	return 0;
}

// Replaces all line separators with a space
void* line_separator_thread() {
	char* line = calloc(INPUT_SIZE + 1, sizeof(char));
	line = get_buff_1();
	
	int size = strlen(line);

	for (int i = 0; i < size; i++) {
		if (line[i] == '\n') {
			line[i] = ' ';
		}
	}
	
	put_buff_2(line);

	return 0;
}

// Replaces every pair of plus signs "++" by a "^"
void* plus_sign_thread() {
	char* line = calloc(INPUT_SIZE + 1, sizeof(char));
	char* new_line = calloc(INPUT_SIZE + 1, sizeof(char));
	line = get_buff_2();
	int size = strlen(line);

	if (strstr(line, "++") == NULL) {
		put_buff_3(line);
		return 0;
	}

	for (int i, j = 0; i < size; i++, j++) {
		if ((line[i] == '+') && (line[i + 1] == '+')) {
			new_line[j] = '^';
			i++;
		}
		else {
			new_line[j] = line[i];
		}
	}
	new_line[strlen(new_line)] = '\0';

	put_buff_3(new_line);

	return 0;
}

// Writes proccessed data to standard output as lines of exactly 80 characters
void* output_thread() {
	char* line = calloc(INPUT_SIZE + 1, sizeof(char));
	line = get_buff_3();
	int size = strlen(line);

	for (int i, j = 0; i < size; i++, j++) {
		printf("%c", line[i]);
		fflush(stdout);
		if (j >= OUTPUT_SIZE) {
			printf("\n");
			fflush(stdout);
			j = 0;
		}
	}
	
	printf("\n");
	fflush(stdout);

	return 0;
}

int main() {
	pthread_mutex_init(&mutex_1, NULL);
	pthread_mutex_init(&mutex_2, NULL);
	pthread_mutex_init(&mutex_3, NULL);

	pthread_t threads[NUM_THREADS];

	pthread_create(&threads[0], NULL, input_thread, NULL); 
	pthread_create(&threads[1], NULL, line_separator_thread, NULL); 
	pthread_create(&threads[2], NULL, plus_sign_thread, NULL); 
	pthread_create(&threads[3], NULL, output_thread, NULL); 

	pthread_join(threads[0], NULL);
	pthread_join(threads[1], NULL);
	pthread_join(threads[2], NULL);
	pthread_join(threads[3], NULL);
	
	return 0;
}

