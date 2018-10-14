#include <stdio.h>
#include <zconf.h>
#include <wait.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <errno.h>

//functions prototyping
char *readUserInput();

void displayWelcomeScreen();

void printCurrentDirectory();

char **tokenize(char *str, int *);

void runCommand(char *tokens[], int);

void showHelp();

void handle_sigchld(int sig);

void executeBackground(char *tokens[], int numberOfTokens);

void executeBlocking(char *tokens[]);

int main() {

    //show start up screen
    displayWelcomeScreen();

    //pointer to input string
    char *pUserInputStr;

    //array of (char*) to store strings(tokens of input string split by " ")
    char **tokens;

    //to store number of tokens in the user input string
    int numberOfTokens;


    do {

        //displays the cwd then waits for a command
        printf("\n");
        printCurrentDirectory();
        fputs(" >> ", stdout);
        pUserInputStr = readUserInput();

        //handles empty command
        if (strlen(pUserInputStr) == 0) {
            puts("Please enter a command, enter help for displaying help.");
            continue;
        }

        tokens = tokenize(pUserInputStr, &numberOfTokens);

        runCommand(tokens, numberOfTokens);

    } while (1);

}

/**
 * executes given command
 *
 * @param tokens
 */
//When a child process terminates it does not disappear entirely.
//Instead it becomes a ‘zombie process’ which is no longer capable of executing,
//but which still has a PID and an entry in the process table.
//This is indicated by the state code Z in ps or top.
//The presence of a moderate number of zombie processes is not particularly harmful,
//but they add unnecessary clutter that can be confusing to the administrator.
//In extreme cases they could exhaust the number of available process table slots.
//For these reasons, well-behaved programs should ensure that
//zombie processes are removed in a timely manner.
//The process of eliminating zombie processes is known as ‘reaping’.
//The simplest method is to call wait, but this will block the parent process
//if the child has not yet terminated.
// Alternatives are to use waitpid to poll or SIGCHLD to reap asynchronously.
void runCommand(char *tokens[], int numberOfTokens) {

    //checks if it is a built in command first (exit,help,cd)
    if (strcmp(tokens[0], "exit") == 0)
        exit(0);
    else if (strcmp(tokens[0], "help") == 0)
        showHelp();
    else if (strcmp(tokens[0], "cd") == 0)
        chdir(tokens[1]);
    else {

        //decides weather to execute in background or to block parent
        //process until child terminates
        if (strcmp(tokens[numberOfTokens - 1], "&") == 0)//if command ends with & then exec in background
            executeBackground(tokens, numberOfTokens);
        else
            executeBlocking(tokens);

    }

}

/**
 * runs command in background child process
 *
 * @param tokens
 * @param numberOfTokens
 */
void executeBackground(char *tokens[], int numberOfTokens) {

    //removes the & from tokens list by making it NULL
    tokens[numberOfTokens - 1] = NULL;

    //registering a signal handler using the sigaction function
    struct sigaction sa;
    sa.sa_handler = &handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, 0) == -1) {
        perror(0);
        exit(1);
    }

    // create a new child process using fork()
    int pid = fork();

    if (pid == -1) {
        printf("\nFailed forking child.");
        return;
    } else if (pid == 0) {//child
        if (execvp(tokens[0], tokens) < 0) {
            printf("\nCommand couldn't be executed.");
        }
    }

}

/**
 * normal blocking execution of commands
 *
 * @param tokens
 */
void executeBlocking(char *tokens[]) {
    // create a new child process using fork()
    int pid = fork();

    if (pid == -1) {
        printf("\nFailed forking child.");
        return;
    } else if (pid == 0) {//child
        if (execvp(tokens[0], tokens) < 0) {
            printf("\nCommand couldn't be executed.");
        }
    } else {//parent

        // blocking until child terminates
        wait(NULL);
        return;
    }
}

/**
 * displays a help screen
 */
void showHelp() {
    puts("\n***WELCOME TO MY Simple Shell HELP********************"
         "\n*Copyright @ Moamen Mohamed                          *"
         "\n*List of Commands supported                          *"
         "\n*--------------------------                          *"
         "\n*>exit                                               *"
         "\n*>help                                               *"
         "\n*>cd                                                 *"
         "\n*>all other general commands available in UNIX shell *"
         "\n*>no pipe handling                                   *"
         "\n******************************************************");
}

void displayWelcomeScreen() {
    puts("*******************************\n"
         "**********HELLO SHELL**********\n"
         "*******************************\n"
         "Built in commands (exit , help , cd)\n");
}

/**
 * splits a string and allocates space dynamically,
 * delimiter is space
 *
 * @param str : pointer to the string to be split
 *
 * @return char* (string array) of tokens
 */
char **tokenize(char *str, int *pNumberOfTokens) {

    char **tokens = (char **) calloc(2, sizeof(char *));
    int currentTokenSize = 2;
    char *token;

    /* get the first token */
    token = strtok(str, " ");

    int i = 0;

    /* walk through other tokens */
    while (token != NULL) {

        tokens[i++] = token;

        token = strtok(NULL, " ");

        //allocate more space if needed
        if (i == currentTokenSize - 1) {
            currentTokenSize += 1;
            tokens = (char **) reallocarray(tokens, currentTokenSize, sizeof(char *));
        }
    }

    //terminate array by adding NULL at the end
    tokens[i] = NULL;

    //updating data pointed by the pointer to store number of tokens
    *pNumberOfTokens = i;

    return tokens;
}

/**
 * displays current working directory
 * ex home/user/Desktop
 *
 */
void printCurrentDirectory() {

    //PATH_MAX --> max char size in path 4096
    char cwd[PATH_MAX];

    //getcwd() method gets path name of the current working directory stores it in cwd
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s", cwd);
    } else {
        perror("getcwd() error");
    }
}

/**
 * reads user input dynamically by allocating extra
 * space when needed
 *
 * @return char* (string) user entered
 */
char *readUserInput() {

    unsigned int len_max = 10;//initial size
    unsigned int current_size = 0;

    char *pString = malloc(len_max);
    current_size = len_max;

    if (pString != NULL) {
        int c;
        unsigned int i = 0;
        //accept user input until hit enter or end of file
        while ((c = getchar()) != '\n' && c != EOF) {
            pString[i++] = (char) c;

            //if i reached maximize size then realloc size (increase size)
            if (i == current_size) {
                current_size = i + len_max;
                pString = realloc(pString, current_size);
            }
        }

        pString[i] = '\0'; //terminate by NULL

        return pString;
    }

    return NULL;
}

/**
 * Defines an interrupt handler that handles
 * SIGCHLD.
 * It is called when a child process is terminated
 *
 * @param sig
 */
void handle_sigchld(int sig) {

    int saved_errno = errno;
    while (waitpid((pid_t) (-1), 0, WNOHANG) > 0) {}
    errno = saved_errno;
}

//The reason for calling waitpid as opposed to wait is to allow use of the WNOHANG option,
//which prevents the handler from blocking.
//This allows for the possibility of SIGCHLD being raised for reasons other than
//the termination of a child process.
//SIGCHLD has three conventional uses:
//to indicate that a child process has terminated, stopped or continued.
//The latter two conditions can be suppressed using SA_NOCLDSTOP ,
//but that would not prevent a process with the right permissions from raising SIGCHLD
//for any reason using the kill function or an equivalent.
//The reason for placing waitpid within a loop is to allow for
//the possibility that multiple child processes could terminate
//while one is in the process being reaped.
//Only one instance of SIGCHLD can be queued,
//so it may be necessary to reap several zombie processes during one invocation
//of the handler function.
//The loop ensures that any zombies which existed prior to invocation of the handler
//function will be reaped.
//If any further zombies come into being after that moment in time then
//they may or may not be reaped by that invocation of the handler function (depending on the timing),
//but they should leave behind a pending SIGCHLD that will result in the handler being called again.
//There is a possibility that waitpaid could alter errno.
//Saving errno then restoring it afterwards prevents any change from interfering with
//code outside the handler.

//Resource:http://www.microhowto.info/howto/reap_zombie_processes_using_a_sigchld_handler.html