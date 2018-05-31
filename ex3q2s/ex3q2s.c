#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct ParallelSortableArray {
    size_t size;
    int parallelLevel;
    int *nums;
} ParallelSortableArray;

void myParRecMerge(int *nums, size_t size, int splitLevel);
void myMerge(int *nums, size_t size);
void error(char *msg);
char *getIPAddress(struct sockaddr_in *cli_addr);
ParallelSortableArray getPArrFromFile(const char *fileName);

int main(int argc, char **argv) {
    struct sockaddr_in cli_addr;
    socklen_t clilen;

    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if (sockfd < 0)
        error("ERROR opening socket");

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    ParallelSortableArray pArr = getPArrFromFile(argv[1]);
    size_t length = pArr.size / pArr.parallelLevel;

    listen(sockfd,pArr.parallelLevel);
    int sockfds[pArr.parallelLevel];

    printf("Amount of numbers that sort: %zu\n", pArr.size);
    printf("Degree of parallelism: %d\n", pArr.parallelLevel);
    printf("Before sort: %d", pArr.nums[0]);
    for (int i = 1; i < pArr.size; ++i)
        printf(",%d", pArr.nums[i]);

    for (int i = 0; i < pArr.parallelLevel; ++i) {
        sockfds[i] = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (sockfds[i] < 0)
            perror("ERROR on accept");
        char tmpBuffer[1];
        read(sockfds[i], tmpBuffer, 1);
        write(sockfds[i], pArr.nums + (i * length), length * sizeof(int));

        char *ip = getIPAddress(&cli_addr);
        printf("\nGot request from %s", ip);
        free(ip);

        printf("\nSending '%zu' numbers to socket '%d'", length, sockfds[i]);
        fflush(stdout);
    }
    for (int i = 0; i < pArr.parallelLevel; ++i) {
        int input[length];
        read(sockfds[i], input, length * sizeof(int));
        printf("\nRead from socket: '%d'\n%d", sockfds[i], input[0]);

        for (int j = 1; j < length; ++j) {
            printf(",%d", input[j]);
        }
        for (int j = 0; j < length; ++j) {
            pArr.nums[(i * length) + j] = input[j];
        }
        close(sockfds[i]);
    }

    myParRecMerge(pArr.nums, pArr.size, pArr.parallelLevel);

    printf("\nFinal answer: %d", pArr.nums[0]);
    for (int i = 1; i < pArr.size; ++i)
        printf(",%d", pArr.nums[i]);

    printf("\n");
    close(sockfd);
    free(pArr.nums);
    return 0;
}
/**
 * Helper function to create the ParallelSortableArray struct from a file in the format of
 * <array size>
 * <parallelLevel>
 * <nums> (Array of ints)
 * @param fileName
 * @return ParallelSortableArray
 */
ParallelSortableArray getPArrFromFile(const char *fileName) {
    ParallelSortableArray pArr;
    FILE* fp = fopen(fileName, "r");
    fscanf(fp, "%zu", &pArr.size);
    fscanf(fp, "%d", &pArr.parallelLevel);

    pArr.nums = malloc(pArr.size * sizeof(int));
    fscanf(fp, "%d", &pArr.nums[0]);
    for (int i = 1; i < pArr.size; ++i)
        fscanf(fp, ",%d", &pArr.nums[i]);

    fclose(fp);
    return pArr;
}
/**
 * Helper function to get IP address as a string
 * @param cli_addr
 * @return
 */
char *getIPAddress(struct sockaddr_in *cli_addr) {
    char* ip = malloc(sizeof(char) * INET_ADDRSTRLEN + 1);
    inet_ntop(AF_INET, &cli_addr->sin_addr, ip, INET_ADDRSTRLEN + 1);
    return ip;
}

/**
 *
 * @param nums
 * @param size
 * @param splitLevel
 */
void myParRecMerge(int *nums, size_t size, int splitLevel) {
    if (splitLevel == 1)
        return;
    else {
        myParRecMerge(nums, size / 2, splitLevel / 2);
        myParRecMerge(nums + (size / 2), size / 2, splitLevel / 2);
        myMerge(nums, size);
    }
}
/**
 * Simple merge algorithm (special case of arrays who are of size = x^2)
 * Merges to halves of an array (which are already sorted)
 * @param nums array to merge
 * @param size
 */
void myMerge(int *nums, size_t size) {
    int mergedNums[size];
    size_t i = 0, j = size / 2, k;
    for (k = 0; i < size / 2 && j < size; k++) {
        if (nums[i] < nums[j]) {
            mergedNums[k] = nums[i];
            i++;
        }
        else {
            mergedNums[k] = nums[j];
            j++;
        }
    }
    while (i < size / 2) {
        mergedNums[k] = nums[i];
        i++;
        k++;
    }
    while (j < size) {
        mergedNums[k] = nums[j];
        j++;
        k++;
    }
    for (i = 0; i < size; ++i)
        nums[i] = mergedNums[i];
}
void error(char *msg) {
    perror(msg);
    exit(1);
}
