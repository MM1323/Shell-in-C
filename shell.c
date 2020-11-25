/*****
* Project 04: Shell
* COSC 208, Introduction to Computer Systems, Fall 2020
*****/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//We included:
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>


#define PROMPT "shell> "
#define MAX_BACKGROUND 50

/*
 * Break input string into an array of strings.
 * @param input the string to tokenize
 * @param delimiters the characters that delimite tokens
 * @return the array of strings with the last element of the array set to NULL
 */
char** tokenize(const char *input, const char *delimiters) {
    char *token = NULL;

    // make a copy of the input string, because strtok
    // likes to mangle strings.  
    char *input_copy = strdup(input);

    // find out exactly how many tokens we have
    int count = 0;
    for (token = strtok(input_copy, delimiters); token; 
            token = strtok(NULL, delimiters)) {
        count++ ;
    }
    free(input_copy);

    input_copy = strdup(input);

    // allocate the array of char *'s, with one additional
    char **array = (char **)malloc(sizeof(char *)*(count+1));
    int i = 0;
    for (token = strtok(input_copy, delimiters); token;
            token = strtok(NULL, delimiters)) {
        array[i] = strdup(token);
        i++;
    }
    array[i] = NULL;
    free(input_copy);
    return array;
}

/*
 * Free all memory used to store an array of tokens.
 * @param tokens the array of tokens to free
 */
void free_tokens(char **tokens) {
    int i = 0;
    while (tokens[i] != NULL) {
        free(tokens[i]); // free each string
        i++;
    }
    free(tokens); // then free the array
}


int create_process(char **command) {
    pid_t child_pid;
    int ret;

    child_pid = fork();

	if (child_pid == 0) { 
		ret = execv(command[0], command);//execv only returns -1 if an error has occurred 
		fprintf(stderr, "execv failed: %s\n", strerror(errno)); 
        //might create helper function that handles other types of errors. (Defensive programming);
	}
	else {
		int child_status; 
        waitpid(child_pid, &child_status, 0);
		printf("Child finished\n"); //DEBUGGER
	}
    return child_pid;
}



int main(int argc, char **argv) {
    // main loop for the shell
    printf("%s", PROMPT);
    fflush(stdout);  // Display the prompt immediately
    char buffer[1024];
    while (fgets(buffer, 1024, stdin) != NULL) {
        char **command = tokenize(buffer, " \t\n"); //command[0] holds "/bin/ls"; command[1] holds "-l"
        printf("%s \n", command[0]);//DEBUGGER
        if (strncmp(command[0], "exit", 4) == 0) {  //check whether built-in or exit() command
           exit(0); 
        }
        else {
            int child_pid = create_process(command);
            //exit(child_pid); may not be necessary. Uncommenting it will cause shell to exit while loop
        }
        printf("%s", PROMPT);
        fflush(stdout);  // Display the prompt immediately
    }

    return EXIT_SUCCESS;
}
