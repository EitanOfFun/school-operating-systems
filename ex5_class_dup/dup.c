#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    char fileName[50];
    char message[] = "this output is redirected from standard output to an output file";

    scanf("%s", fileName);
    int fd = open(fileName, O_WRONLY | O_CREAT, S_IRGRP | S_IROTH | S_IRUSR | S_IWGRP | S_IWUSR | S_IWOTH);
    if (fd == -1) {
        perror("failed to open file");
        exit(EXIT_FAILURE);
    }
    int status = dup2(fd, STDOUT_FILENO);
    if (status == -1) {
        perror("dup2 failed\n");
        exit(EXIT_FAILURE);
    }
    printf("%s", message);
    return 0;
}