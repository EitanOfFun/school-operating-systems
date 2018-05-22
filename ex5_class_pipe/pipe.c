#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#define NUMS 5

double avg(const int *nums, int length);

int main() {
    int nums[NUMS];
    for (int i = 0; i < NUMS; ++i)
        scanf("%d", &nums[i]);
    int pipe_fd[2];
    int status = pipe(pipe_fd);
    pid_t pid = fork();

    if (status == -1) {
        if (pid == 0)
            perror("cannot open pipe\n");
        exit(EXIT_FAILURE);
    }

    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0) {
        if (close(pipe_fd[0]) == -1) {
            perror("An error occurred while closing the pipe\n");
            exit(EXIT_FAILURE);
        }
        double average = avg(nums, NUMS);
        write(pipe_fd[1], &average, sizeof(double));
        close(pipe_fd[1]);
        exit(EXIT_SUCCESS);
    } else {
        if (close(pipe_fd[1]) == -1) {
            printf("An error occurred while closing the pipe\n");
            exit(EXIT_FAILURE);
        }
        double average;
        read(pipe_fd[0], &average, sizeof(double));
        printf("The average is: %f\n", average);
        close(pipe_fd[0]);
    }
    return 0;
}

double avg(const int *nums, int length) {
    double average = 0;
    for (int i = 0; i < length; ++i)
        average += nums[i];
    return average / length;
}