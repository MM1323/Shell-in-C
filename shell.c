/*****
* Project 04: Shell
* COSC 208, Introduction to Computer Systems, Fall 2020
*****/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//We included:
#include <sys/wait.h>
#include <sys/signal.h>
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

/*
 * Create process and wait for it to finish
 */
int create_process(char **command) {
    pid_t pid, wpid;
    int status;
    pid = fork();
	if (pid == 0) { 
		execv(command[0], command);//execv only returns -1 if an error has occurred 
		fprintf(stderr, "execv failed: %s\n", strerror(errno)); 
        exit(0); //exit failed process
	} else if (pid < 0) {
        //error forking
        perror("error forking");
    } else {
        //Parent process 
        wpid = waitpid(pid, &status, 0);
	}
    return pid;
}

/*
 * Create process and don't wait
 */
int create_back_process(char **command) {
    pid_t pid;
    int ret;
    pid = fork();
	if (pid == 0) { 
		ret = execv(command[0], command);//execv only returns -1 if an error has occurred 
		fprintf(stderr, "execv failed: %s\n", strerror(errno)); 
        exit(0); //exit failed process
	}
    return pid;
}

/*
 * Remove pid number and shift the rest to replace it
 * @param pid numbers, pid to remove, number of active processes
 */
int * remove_process(int* PIDS_IN_PROCESS, int pid, int num_active) {
    int active_PID_number = num_active;
    int y;
    int found = 0;
    for (y = 0; y < active_PID_number - 1; y++){
        if (pid == PIDS_IN_PROCESS[y]) {
              found = 1;
        }
        if (found == 1) { 
            PIDS_IN_PROCESS[y] = PIDS_IN_PROCESS[y + 1];
        }
    }
    return PIDS_IN_PROCESS;
}

int main(int argc, char **argv) {
    // main loop for the shell
    printf("%s", PROMPT);
    fflush(stdout);  // Display the prompt immediately
    char buffer[1024];
    int active_PID_number = 0; //keep track of # of active processes
    int PIDS_IN_PROCESS[MAX_BACKGROUND];
    while (fgets(buffer, 1024, stdin) != NULL) {
        // check for runnning processes
        for (int i = 0; i < active_PID_number; i++) {
            int child_pid = PIDS_IN_PROCESS[i];
            int child_status = 0;
            int ret = waitpid(child_pid, &child_status, WNOHANG);
            if (ret > 0) { // if finished do print statment
                printf("%d finished with exit code %d\n", child_pid, child_status);
                remove_process(PIDS_IN_PROCESS,child_pid, active_PID_number); // remove ## from the list
                active_PID_number--; // active_PID_number--
            }
            else if (ret == -1) {
                fprintf(stderr, "execv failed: %s\n", strerror(errno)); 
            }
        }
        char **command = tokenize(buffer, " \t\n"); // tokenize input
        //check if nothing was entered
        if (command[0] != NULL){
            int i = 0;
            while(command[i] != NULL) { // Get where the last char is
                i++;
            }
            if (strncmp(command[0], "exit", 4) == 0) {  //check whether built-in or exit() command
                exit(0); 
            } else if (strncmp(command[0], "jobs", 10) == 0) { //print number of jobs
                printf("Process currently active: ");
                int max = active_PID_number - 1;
                for (int x = 0; x < max; x++) {
                    printf("%d, ", PIDS_IN_PROCESS[x]);
                }
                printf("%d\n", PIDS_IN_PROCESS[max]);
            } else if (strncmp(command[0], "kill", 4) == 0) { //kill pid in process
                pid_t child_pid = atoi(command[1]);
                kill(child_pid, SIGKILL); // use kill 
                remove_process(PIDS_IN_PROCESS, child_pid, active_PID_number);
                active_PID_number--; // active_PID_number--
            } else if (strncmp(command[i-1], "&", 4) == 0){
                command[i-1] = NULL; 
                int child_pid = create_back_process(command);
                PIDS_IN_PROCESS[active_PID_number] = child_pid; // add ## to the list (PIDS_IN_PROCESS)
                active_PID_number++; // increase active_PID_number by one
            } else {
                create_process(command);
            }
        }
        printf("%s", PROMPT);
        fflush(stdout);  // Display the prompt immediately
    }

    return EXIT_SUCCESS;
}
