/**
 * Username: eitanst
 * Eitan Sternlicht
 * ID: 204070635
 * Ex2
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <string.h>
#include <assert.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_INPUT 512 // max characters user can input in shell
#define MAX_PATH 512 // max characters a directory path can be

#define COMMAND_NOT_FOUND "command not found"
// todo free memory
typedef struct {
    int readFD;
    int writeFD;
} Pipe;

void printArgs(char **s);
int getWordCount(const char *input);
void setArgs(char **args, const char *input);
void freeArgs(char **args, const int words);
Pipe myPipe();
int getPipeIndex(char ** args);
int getRedirectedIndex(char **args);
bool isPiped(int pipeIndex);
bool isRedirected(int redirectedIndex);
void handler(int signum);

/**
 * main program loop, ends when user enters done
 * @return
 */
int main() {
    signal(SIGINT, SIG_IGN);
    signal(SIGCHLD, handler);


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
                if (args[1] != NULL) // add path to current working directory and navigate to it
                    chdir(args[1]);
            } else {
                commandsLength += strlen(args[0]);
                numOfCommands++;
                int pipeIndex = getPipeIndex(args);
                int redirectedIndex = getRedirectedIndex(args);
                bool bg = strcmp("&", args[words - 1]) == 0;

                if (isPiped(pipeIndex)) {
                    pid_t pid = fork();
                    if (pid == -1)
                        perror("Error creating child process\n");
                    else if (pid == 0) {
                        Pipe p = myPipe();
                        pid_t pid2 = fork();
                        if (pid2 == -1)
                            perror("Error creating child process\n");
                        else if (pid2 == 0) {
                            close(p.readFD);
                            free(args[pipeIndex]);
                            args[pipeIndex] = NULL;
                            dup2(p.writeFD, STDOUT_FILENO);
                            execvp(args[0], args);
                            exit(EXIT_SUCCESS);
                        } else {
                            wait(NULL);
                            close(p.writeFD);
                            dup2(p.readFD, STDIN_FILENO);

                            if (isRedirected(redirectedIndex)) {
                                int fd = STDOUT_FILENO;
                                if (strcmp(">", args[redirectedIndex]) == 0) {
                                    fd = open(args[redirectedIndex + 1], O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                                    dup2(fd, STDOUT_FILENO);
                                } else if (strcmp(">>", args[redirectedIndex]) == 0) {
                                    fd = open(args[redirectedIndex + 1], O_CREAT |O_APPEND |O_WRONLY ,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                                    dup2(fd, STDOUT_FILENO);
                                } else if (strcmp("2>", args[redirectedIndex]) == 0) {
                                    fd = open(args[redirectedIndex + 1], O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                                    dup2(fd, STDERR_FILENO);
                                } else if (strcmp("<", args[redirectedIndex]) == 0) {
                                    fd = open(args[redirectedIndex + 1], O_RDONLY, 0);
                                    dup2(fd, STDIN_FILENO);
                                }
                                free(args[redirectedIndex]);
                                args[redirectedIndex] = NULL;
                            }
                            printArgs(args + pipeIndex + 1);
                            execvp(args[pipeIndex + 1], args + pipeIndex + 1);
                            exit(EXIT_SUCCESS);
                        }
                    } else {
                        waitpid(pid, NULL, 0);
                    }

                } else if (isRedirected(redirectedIndex)) {
                    pid_t pid = fork();
                    if (pid == -1) {
                        perror("Error creating child process\n");
                    } else if (pid == 0) {
                        int fd = STDOUT_FILENO;
                        if (strcmp(">", args[redirectedIndex]) == 0) {
                            fd = open(args[redirectedIndex + 1], O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                            dup2(fd, STDOUT_FILENO);
                        } else if (strcmp(">>", args[redirectedIndex]) == 0) {
                            fd = open(args[redirectedIndex + 1], O_CREAT |O_APPEND |O_WRONLY ,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                            dup2(fd, STDOUT_FILENO);
                        } else if (strcmp("2>", args[redirectedIndex]) == 0) {
                            fd = open(args[redirectedIndex + 1], O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                            dup2(fd, STDERR_FILENO);
                        } else if (strcmp("<", args[redirectedIndex]) == 0) {
                            fd = open(args[redirectedIndex + 1], O_RDONLY, 0);
                            dup2(fd, STDIN_FILENO);
                        }
                        free(args[redirectedIndex]);
                        args[redirectedIndex] = NULL;
                        execvp(args[0], args);
                    } else {
                        wait(NULL);
                    }
                } else {
                    // no redirection and no pipe (normal ex1 kind of command)
                    if (bg) {
                        free(args[words - 1]);
                        args[words - 1] = NULL;
                    }
                    pid_t pid = fork();
                    if (pid == -1) {
                        perror("Error creating child process\n");
                    } else if (pid == 0) {
                        signal(SIGINT, SIG_DFL);
                        if (execvp(args[0], args) == -1)
                            printf("%s: %s\n", args[0], COMMAND_NOT_FOUND);
                        freeArgs(args, words);
                        exit(EXIT_SUCCESS);
                    } else if (!bg) {
                        pause();
                    }
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

    for (int i = 0; i < words; ++i) {
        if (args[i] != NULL)
            free(args[i]);
    }

}
Pipe myPipe() {
    int pipe_desc[2];
    if (pipe(pipe_desc) == -1) {
        perror("cannot open pipe");
        exit(EXIT_FAILURE) ;
    }
    Pipe p;
    p.readFD = pipe_desc[0];
    p.writeFD = pipe_desc[1];
    return p;
}
int getPipeIndex(char ** args) {
    for (int i = 0; args[i] != NULL; ++i)
        if (strcmp(args[i], "|") == 0)
            return i;
    return -1;
}
int getRedirectedIndex(char **args) {
    for (int i = 0; args[i] != NULL; ++i)
        if (args[i][0] == '>' || args[i][0] == '<' || (args[i][1] == '>' && isdigit(args[i][0]))) {
            if (strcmp(args[i], ">") == 0 || strcmp(args[i], ">>") == 0 || strcmp(args[i], "<") == 0 || strcmp(args[i], "2>") == 0)
                return i;
            else
                return -1;
        }
    return -1;
}
void printArgs(char **args) {
    printf("[");
    if (args == NULL || args[0] == NULL)
        printf("]");
    else {
        for (int i = 0; args[i] != NULL; ++i)
            printf("%s, ", args[i]);
        printf("]");
    }
    fflush(stdout);
}
bool isPiped(int pipeIndex) {
    return pipeIndex != -1;
}
bool isRedirected(int redirectedIndex) {
    return redirectedIndex != -1;
}
void handler(int signum) {
    signal(SIGINT, handler);
    if (signum == SIGCHLD) {
        signal(SIGINT, handler);
    }
}
