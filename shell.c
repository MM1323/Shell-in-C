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
        exit(0); //exit failed process
	}
	else {
		int child_status; 
        waitpid(child_pid, &child_status, 0);
	}
    return child_pid;
}

int create_back_process(char **command) {
    pid_t child_pid;
    int ret;

    child_pid = fork();

	if (child_pid == 0) { 
		ret = execv(command[0], command);//execv only returns -1 if an error has occurred 
		fprintf(stderr, "execv failed: %s\n", strerror(errno)); 
        //might create helper function that handles other types of errors. (Defensive programming);
        exit(0); //exit failed process
	}
    return child_pid;
}



int main(int argc, char **argv) {
    // main loop for the shell
    printf("%s", PROMPT);
    fflush(stdout);  // Display the prompt immediately
    char buffer[1024];
    unsigned int active_PID_number = 0; //keep track of # of active processes
    unsigned int PIDS_IN_PROCESS[MAX_BACKGROUND]; //Moved here b/c needs to be declared before check
    while (fgets(buffer, 1024, stdin) != NULL) {
        for (int i = 0; i < active_PID_number; i++) {// check for runnning processes
            int child_pid = PIDS_IN_PROCESS[i];
            int child_status;
            int ret = waitpid(child_pid, &child_status, WNOHANG);
            if ( ret > 0) {// if finished do print statment
                printf("%d finished with exit code %d\n", child_pid, child_status);
                // remove ## from the list
                active_PID_number--;// active_PID_number--
            }
            else if (ret == -1) {
                fprintf(stderr, "execv failed: %s\n", strerror(errno)); 
            }
        }
        char **command = tokenize(buffer, " \t\n"); //command[0] holds "/bin/ls"; command[1] holds "-l"
        printf("Beginning is %s \n", command[0]);//DEBUGGER
        /*Moved these declarations outside while loop;
        unsigned int PIDS_IN_PROCESS[MAX_BACKGROUND]; 
        unsigned int active_PID_number = 0;*/

        //check if nothing was entered
        if (command[0] != NULL){
            // Get what the last char is
            int i = 0;
            while(command[i] != NULL) {
                i++;
            }
            printf("The last char is %s\n", command[i-1]);
            ///////////////////////////
            if (strncmp(command[0], "exit", 4) == 0) {  //check whether built-in or exit() command
                exit(0); 
            } else if (strncmp(command[0], "jobs", 4) == 0) { //print number of jobs
                // printf("ITS JOBS\n"); //DEBUGGER
                // This should work if the bottom is set
                // Need to work in out to include the ,
                printf("Process currently active: ");
                for (int x = 0; x < (active_PID_number - 1); x++) {
                    printf("%d, ", PIDS_IN_PROCESS[x]);
                }
                printf("%d\n", PIDS_IN_PROCESS[active_PID_number - 1]);
            } else if (strncmp(command[0], "kill", 4) == 0) { //kill pid in process
                printf("ITS KILL PID\n");
                // use kill + extras found in instr
                // take ## out of the list
                // active_PID_number--
            } else if (strncmp(command[i-1], "&", 4) == 0){
                printf("ITS AND &\n"); //DEBUGGER
                printf("Active process count before %s is created: %d\n", command[0], active_PID_number);  //DEBUGGER
                int child_pid = create_back_process(command);
                printf("New PID: %d\n", child_pid); //DEBUGGER
                PIDS_IN_PROCESS[active_PID_number] = child_pid;// add ## to the list (PIDS_IN_PROCESS)
                active_PID_number++;// increase active_PID_number by one
                printf("Active process count after %s is created: %d\n", command[0], active_PID_number);  //DEBUGGER

            } else {
                // printf("IS NOT NULL \n"); //DEBUGGER
                int child_pid = create_process(command);
                //might have to declare child status variable here
            }
        }

        printf("%s", PROMPT);
        fflush(stdout);  // Display the prompt immediately
    }

    return EXIT_SUCCESS;
}
