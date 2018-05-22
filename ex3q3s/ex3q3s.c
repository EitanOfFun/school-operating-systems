#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MSG_LEN 255

typedef struct ParallelSortableArray {
    int size;
    int parallelLevel;
    int *nums;
} ParallelSortableArray;

void myRecMerge(int *nums, int size, int splitLevel, int startIndex);
void myMerge(int *nums, int size);
void error(char *msg);
ParallelSortableArray getPArrFromFile(const char *fileName) {
    ParallelSortableArray pArr;
    FILE* fp = fopen(fileName, "r");
    fscanf(fp, "%d", &pArr.size);
    fscanf(fp, "%d", &pArr.parallelLevel);
    
    pArr.nums = malloc(pArr.size * sizeof(int));
    fscanf(fp, "%d", &pArr.nums[0]);
    for (int i = 1; i < pArr.size; ++i)
        fscanf(fp, ",%d", &pArr.nums[i]);
    
    fclose(fp);
    return pArr;
}
char *getIPAddress(struct sockaddr *cli_addr) {
    struct sockaddr_in* pV4Addr = (struct sockaddr_in*)cli_addr;
    struct in_addr ipAddr = pV4Addr->sin_addr;
    char* ip = malloc(sizeof(char) * INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &ipAddr, ip, INET_ADDRSTRLEN);
    return ip;
}
int main(int argc, char **argv) {
    struct sockaddr cli_addr;
    socklen_t clilen;

    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if (sockfd < 0)
        error("ERROR opening socket");

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    ParallelSortableArray pArr = getPArrFromFile(argv[2]);
    int length = pArr.size / pArr.parallelLevel;

    listen(sockfd,pArr.parallelLevel);
    int sockfds[pArr.parallelLevel];
    printf("Amount of numbers that sort: %d\n", pArr.size);
    printf("Degree of parallelism: %d\n", pArr.parallelLevel);
    printf("Before sort: %d", pArr.nums[0]);
    for (int i = 1; i < pArr.size; ++i)
        printf(",%d", pArr.nums[i]);

    for (int i = 0; i < pArr.parallelLevel; ++i) {
        sockfds[i] = accept(sockfd, &cli_addr, &clilen);
        if (sockfds[i] < 0)
            perror("ERROR on accept");
        char tmpBuffer[1];
        read(sockfds[i], tmpBuffer, 1);
        write(sockfds[i], pArr.nums + (i * length), length * sizeof(int));

        char *ip = getIPAddress(&cli_addr);
        printf("\nGot request from %s", ip);
        free(ip);

        printf("\nSending '%d' numbers to socket '%d'", length, sockfds[i]);
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

    myRecMerge(pArr.nums, pArr.size, pArr.parallelLevel, 0);

    printf("\nFinal answer: %d", pArr.nums[0]);
    for (int i = 1; i < pArr.size; ++i)
        printf(",%d", pArr.nums[i]);

    printf("\n");
    close(sockfd);
    free(pArr.nums);
    return 0;
}
void myRecMerge(int *nums, int size, int splitLevel, int startIndex) {
    if (splitLevel == 1)
        return;
    else {
        myRecMerge(nums, size / 2, splitLevel / 2, startIndex);
        myRecMerge(nums, size / 2, splitLevel / 2, startIndex + (size / 2));
        myMerge(nums + startIndex, size);
    }
}
void myMerge(int *nums, int size) {
    int mergedNums[size];
    int i = 0, j = size / 2, k;
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