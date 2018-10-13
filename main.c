#include <stdio.h>
#include <zconf.h>
#include <wait.h>
#include <string.h>
#include <malloc.h>

//functions prototyping
void readUserInput(char *pUserInputStr);

void displayWelcomeScreen();

void printCurrentDir();

int main() {

    displayWelcomeScreen();
    printCurrentDir();


    char *pUserInputStr;
    readUserInput(pUserInputStr);

    while (strcmp(pUserInputStr, "exit") != 0) {

        char *token = strtok(pUserInputStr, " ");
        char *command = token;

        char *params = malloc(0);
        while (token != NULL) {
            realloc(params, (sizeof(params) + sizeof(token)));
            strcpy(params, token);

            token = strtok(NULL, "-");
        }

        int processId = fork();

        if (processId == 0)
            execvp(command, params);

        wait(NULL);


        //free pointer to user input
        free(pUserInputStr);
        pUserInputStr = NULL;

        //read next user input
        readUserInput(pUserInputStr);
    }


    return 0;
}

void printCurrentDir() {

}

void displayWelcomeScreen() {
    puts("*******************************");
    puts("*******************************");
    puts("**********HELLO SHELL**********");
    puts("*******************************");
    puts("*******************************");
    puts(">>");

}

void readUserInput(char *pUserInputStr) {
    unsigned int len_max = 10;
    unsigned int current_size = 0;

    pUserInputStr = malloc(len_max);
    current_size = len_max;

    if (pUserInputStr != NULL) {
        int c = EOF;
        unsigned int i = 0;
        //accept user input until hit enter or end of file
        while ((c = getchar()) != '\n' && c != EOF) {
            pUserInputStr[i++] = (char) c;

            //if i reached maximize size then realloc size
            if (i == current_size) {
                current_size = i + len_max;
                pUserInputStr = realloc(pUserInputStr, current_size);
            }
        }

        pUserInputStr[i] = '\0';
    }
}
