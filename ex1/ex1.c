/**
 * Username: eitanst
 * Eitan Sternlicht
 * ID: 204070635
 * Ex1
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <string.h>
#include <assert.h>
#include <sys/wait.h>

#define MAX_INPUT 512 // max characters user can input in shell
#define MAX_PATH 512 // max characters a directory path can be

#define COMMAND_NOT_FOUND "command not found"

int getWordCount(const char *input);
void setArgs(char **args, const char *input);
void freeArgs(char **args, const int words);


/**
 * main program loop, ends when user enters done
 * @return
 */
int main() {
    int numOfCommands = 0;  // command counter to be printed before termination
    int commandsLength = 0; // command length counter to be printed before termination
    char wd[MAX_PATH]; // working directory

    printf("%s@%s>", getpwuid(0)->pw_name, getcwd(wd, MAX_PATH)); // print shell
    char input[MAX_INPUT];
    fgets(input, MAX_INPUT, stdin);

    while (strcmp("done\n", input) != 0) {
        int words = getWordCount(input); // returns number of args separated by spaces in input
        char *args[words + 1]; // one extra place for NULL at the end
        setArgs(args, input); // parses input and sets args array

        if (strcmp("\n", input) != 0) {
            if (words == 0)
                printf(": %s\n", COMMAND_NOT_FOUND);
            else if (strcmp("cd", args[0]) == 0) {
                if (args[1] != NULL) { // add path to current working directory and navigate to it
                    chdir(args[1]);
                }
            } else {
                commandsLength += strlen(args[0]);
                numOfCommands++;
                pid_t pidOfChild = fork();

                if (pidOfChild == -1) {
                    perror("Error creating child process\n");
                } else if (pidOfChild == 0) {
                    if (execvp(args[0], args) == -1)
                        printf("%s: %s\n", args[0], COMMAND_NOT_FOUND);
                    freeArgs(args, words);
                    exit(EXIT_SUCCESS);
                } else {
                    wait(NULL);
                }
            }
        }
        freeArgs(args, words); // free arg array
        printf("%s@%s>", getpwuid(0)->pw_name, getcwd(wd, MAX_PATH)); //print shell
        fgets(input, MAX_INPUT, stdin); // get input
    }
    printf("Num of cmd: %d\nCmd length: %d\nBye !\n", numOfCommands, commandsLength); // print summary
    return EXIT_SUCCESS;
}
/**
 * helper function which returns count of words separated by a space or \n
 * @param input
 * @return
 */
int getWordCount(const char *input) {
    char s[MAX_INPUT];
    strcpy(s, input);

    char* ptr = strtok(s, " \n");

    int i;
    for (i = 0; ptr != NULL; i++)
        ptr = strtok(NULL, " \n");
    return i;
}
/**
 * helper function to allocate and set args array with input
 * @param args
 * @param input string of unparsed args
 */
void setArgs(char **args, const char *input) {
    char s[MAX_INPUT];
    strcpy(s, input);

    char *ptr = strtok(s, " \n");
    int i;
    for (i = 0; ptr != NULL; ++i) {
        size_t length = strlen(ptr) + 1;
        args[i] = malloc(sizeof(char) * length);
        assert(args[i] != NULL);
        strcpy(args[i], ptr);
        ptr = strtok(NULL, " \n");
    }
    args[i] = NULL;
}
/**
 * helper function to free dyn allocated memory of args array
 * @param args
 * @param words
 */
void freeArgs(char **args, const int words) {
    for (int i = 0; i < words; ++i)
        free(args[i]);
}