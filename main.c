
// Vincent Bell
// CS 344
// Assignment 3: Smallsh
// 2/4/21

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
#include <stdbool.h>
#include <signal.h>
#include <sys/wait.h>

#define MAXCHAR 2048
#define MAXARG 512

char* get_user_input();
struct command* parse_cmd_line(char *cmd_line);
void print_cmd(struct command* cmd);
char* exp_of_var(char* cmd_line);
void run_built_in(struct command *cmd, int *bg_procs, int *bg_count);
char* run_cd(char *path);
void run_status();
void run_exit();
void otherCmd(struct command* cmd, int* bg_procs, int* bg_count);
void input_redirection(struct command* cmd);
void output_redirection(struct command* cmd);
void signalHandling();

struct command {
	char* cmd;
	char** args;
	int nargs;
	char* out;
	char* in;
	int bg;
};

// Prompts the user by displaying ": ". 
// The function calls getline() to store the
// input of the user in char* lineptr and returns
// the input.
char* get_user_input() {
	char* lineptr;
	size_t n = 0;
	int readLine;

	printf(": ");
	fflush(stdout); // flush out output buffers

	readLine = getline(&lineptr, &n, stdin);
	if (readLine == -1) {
		printf("error\n");
	}
	return lineptr;
}

// This function is used for debugging only.
// Prints out the contents of struct comnnand.
void print_cmd(struct command* cmd) {
	printf("cmd: %s\n", cmd->cmd);

	int count = 0;
	printf("args: \n");
	for (int i = 0; cmd->args[i] != NULL; i++) {
		count++;
		printf("%s\n", cmd->args[i]);
	}

	printf("num args: %d\n", cmd->nargs);

	if (cmd->in != NULL) {
		printf("in: %s\n", cmd->in);
	}
	\
	if (cmd->out != NULL) {
		printf("out: %s\n", cmd->out);
	}

	printf("bg: %d\n", cmd->bg);
	fflush(stdout);

}

// Parses through the command line by storing the user input 
// into struct command. Starts out by initializing each 
// value in the struct to NULL. A token is used to seperate 
// each value in the command and is used to find which values 
// are input or output commands.
struct command* parse_cmd_line(char* cmd_line) {

	struct command* parsed_cmd = malloc(sizeof(struct command));

	// Initialize struct
	parsed_cmd->cmd = NULL;
	parsed_cmd->args = NULL;
	parsed_cmd->nargs = 0;
	parsed_cmd->out = NULL;
	parsed_cmd->in = NULL;
	parsed_cmd->bg = 0;

	//init_cmd_struct(parsed_cmd);

	// Allocate space for argsList
	char** argsList = malloc(MAXARG * sizeof(*argsList));

	char* saveptr;
	int num_args = 1;

	// Each command is separated by " " and is stored in cmd
	char* token = strtok_r(cmd_line, " ", &saveptr);
	parsed_cmd->cmd = calloc(strlen(token) + 1, sizeof(char));
	strcpy(parsed_cmd->cmd, token);

	argsList[0] = calloc(strlen(token) + 1, sizeof(char));
	strcpy(argsList[0], token);

	token = strtok_r(NULL, " ", &saveptr);
	// If the input or output token is found, then the next arg 
	// is stored in out or in
	while (token) {
		if ((token[0] == '<') & (strlen)(token) == 1) {
			token = strtok_r(NULL, " ", &saveptr);
			parsed_cmd->in = calloc(strlen(token) + 1, sizeof(char));
			strcpy(parsed_cmd->in, token);
		}
		else if ((token[0] == '>') & (strlen)(token) == 1) {
			token = strtok_r(NULL, " ", &saveptr);
			parsed_cmd->out = calloc(strlen(token) + 1, sizeof(char));
			strcpy(parsed_cmd->out, token);
		}
		else {
			argsList[num_args] = calloc(strlen(token) + 1, sizeof(char));
			strcpy(argsList[num_args], token);
			num_args++;
		}
		token = strtok_r(NULL, " ", &saveptr);
	}

	// If the last arg is "&", then bg is set to 1.
	if (num_args > 1) {
		if (!strcmp(argsList[num_args - 1], "&")) {
			parsed_cmd->bg = 1;
			free(argsList[num_args - 1]);
			num_args--;
		}
	}

	parsed_cmd->args = argsList;
	parsed_cmd->nargs = num_args;

	return parsed_cmd;
}

// Performs variable expansion of "$$".
// Everytime $$ is in the command line it
// will be replaced by the PID.
char* exp_of_var(char* cmd_line) {
	pid_t process = getpid();
	char* pid = calloc(sizeof(char), 50);
	sprintf(pid, "%d", process);

	char* tmp = cmd_line;

	char* exp_cmd = (char*)calloc(MAXCHAR, sizeof(char));

	char* p = strstr(tmp, "$$");

	// Replace "$$" with process id
	while (p != NULL) {
		strncat(exp_cmd, tmp, (strlen(tmp) - strlen(p)));
		strcat(exp_cmd, pid);
		tmp += strlen(tmp) - strlen(p) + strlen("$$");
		p = strstr(tmp, "$$");
	}

	strncat(exp_cmd, tmp, strlen(tmp) - 1);
	free(pid);
	return exp_cmd;
}

// Runs the built in commands exit, cd, status, and calls otherCmd
// if any other commands are used.
void run_built_in(struct command* cmd, int* bg_procs, int* bg_count) {
	if (!strcmp(cmd->cmd, "cd")) {
		//cd is only arg in args
		if (cmd->nargs == 1) {
			run_cd(getenv("HOME"));
		}
		else {
			run_cd(cmd->args[1]);
		}
	}
	else if (!strcmp(cmd->cmd, "status")) {
		run_status();
	}
	else if (!strcmp(cmd->cmd, "exit")) {
		run_exit();
	}
	else {
		otherCmd(cmd, bg_procs, bg_count);
	}
}

// Runs the change directory command cd.
char* run_cd(char* path) {
	// Error condition
	if (chdir(path) == -1){
		printf("Directory could not be found.\n");
		fflush(stdout);
	}
	char* cwd;
	char* buffer = NULL;
	size_t len = 0;
	// Use getcwd() to change directory to path
	cwd = getcwd(buffer, len);
	printf("cwd after change is: %s\n", cwd);
	free(cwd);
}

// Prints out either the exit status or the terminating
// signal of the last foreground process ran by the shell.
void run_status() {
	pid_t process = getpid();
	char* pid = calloc(sizeof(char), 50);
	sprintf(pid, "%d", process);
	int cpid = 0;

	if (WIFEXITED(cpid)) {
		printf("Child %d exited normally with status %d\n", process, WEXITSTATUS(cpid));
	}
	else {
		printf("Child %d exited abnormally due to signal %d\n", process, WTERMSIG(cpid));
	}
	fflush(stdout);
}

// runs the exit command by terminating the program.
void run_exit() {
	kill(0, SIGKILL);
}

// Executes any other command entered in by the user. This is done
// by the parent forking off a child. child will then call execvp
// to run the command
void otherCmd(struct command* cmd, int* bg_procs, int* bg_count) {
	pid_t cpid = fork();
	int cStatus;

	// If in bg mode, add child PID to bg_procs
	if (cmd->bg) {
		bg_procs[*bg_count] = cpid;
		*bg_count += 1;
	}

	switch (cpid) {
	// Error case
	case -1:
		printf("failed to execute %s\n", cmd->cmd);
		fflush(stdout);
		exit(1);
		break;
	// Child process
	case 0:
		// Set up input and output redirection
		input_redirection(cmd);
		output_redirection(cmd);
		// Run new command with execvp()
		execvp(cmd->args[0], cmd->args);
		fprintf(stderr, "error executing %s \n", cmd->cmd);
		exit(1);
		// Parent process
	default:
		if (cmd->bg) {
			fflush(stdout);
			break;
		}
		else {
			waitpid(cpid, &cStatus, 0);
			break;
		}
	}
}

// Performs input redirection with the use of dup2().
// Input files will be redirected using stdin and will
// be opened for reading only.
void input_redirection(struct command* cmd) {
	if (cmd->in != NULL) {
		// Open in input file for read only
		int inputFD = open(cmd->in, O_RDONLY);
		if (inputFD == -1) {
			printf("Can't open %s\n", cmd->in);
			fflush(stdout);
			exit(1);
		}
		int result = dup2(inputFD, 0);
		if (inputFD == -1) {
			printf("Error opening input file \n");
			fflush(stdout);
			exit(1);
		}
	}
	// If the user doesn't redirect stdin for a bg command,
	// then stdin should be redirected to /dev/null
	if (cmd->bg & !cmd->in) {
		int inputFD = open("/dev/null", O_RDONLY);
		int result = dup2(inputFD, 0);
	}	
}

// Performs output redirection with the use of dup2().
// Output files will be redirected usin stdout and should
// opened for writing only.
void output_redirection(struct command* cmd) {
	if (cmd->out != NULL) {
		// Open an output file for read and write only
		int outputFD = open(cmd->out, O_WRONLY | O_CREAT | O_TRUNC, 0666);
		if (outputFD == -1) {
			printf("Can't open %s\n", cmd->out);
			fflush(stdout);
			exit(1);
		}
		int result = dup2(outputFD, 0);
		if (outputFD == -1) {
			printf("Error opening output file \n");
			fflush(stdout);
			exit(1);
		}
	}
	// If the user doesn't redirect stdout for a bg command,
	// then stdout should be redirected to /dev/null
	if (cmd->bg & !cmd->out) {
		int outputFD = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0666);
		int result = dup2(outputFD, 0);
	}
}

// Performs signal handling
void signalHandling() {
	struct sigaction ignore_sig = { 0 };
	ignore_sig.sa_handler = SIG_IGN;
	sigaction(SIGTSTP, &ignore_sig, NULL);
	sigaction(SIGINT, &ignore_sig, NULL);
}

int main() {
	char* cmdString = NULL;
	struct command* parsed_cmd;	
	int *bg_procs = calloc(128, sizeof(int));
	int bg_count = 0;

	signalHandling();

	while (true) {
		cmdString = get_user_input();
		// re-prompt if cmdString is empty or if it's a comment
		while ((strlen(cmdString) == 1) || (cmdString[0] == '#')) {
			cmdString = get_user_input();
		}

		cmdString = exp_of_var(cmdString);
		parsed_cmd = parse_cmd_line(cmdString);
		//print_cmd(parsed_cmd);
		run_built_in(parsed_cmd, bg_procs, &bg_count);
	}
	
	return 0;
}