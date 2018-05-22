#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MSG_LEN 255


void myRecMerge(int *nums, int size, int splitLevel, int startIndex);
void myMerge(int *nums, int size);
void error(char *msg);
int digits(int num);
void stringify(int *nums, int size, char* output);
int chars(int* nums, int size);
void saveToArray(const char *string, int size, int *nums);


int main(int argc, char **argv) {
    int size, splitLevel;
    FILE* fp = fopen(argv[2], "r");
    fscanf(fp, "%d", &size);
    fscanf(fp, "%d", &splitLevel);
    int nums[size], i;
    for (i = 0; i < size - 1; ++i)
        fscanf(fp, "%d,", &nums[i]);
    fscanf(fp, "%d", &nums[i]);
    int length = size / splitLevel;

    char buffer[MSG_LEN];
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

    listen(sockfd,splitLevel);
    int sockfds[splitLevel];

    printf("Amount of numbers that sort: %d\n", size);
    printf("Degree of parallelism: %d\n", splitLevel);
    printf("Before sort: %d", nums[0]);
    for (i = 1; i < size; ++i)
        printf(",%d", nums[i]);

    for (i = 0; i < splitLevel; ++i) {
        sockfds[i] = accept(sockfd, &cli_addr, &clilen);
        if (sockfds[i] < 0)
            perror("ERROR on accept");

        read(sockfds[i], buffer, MSG_LEN);

        int outLen = chars(nums + (i * length), length);
        char output[outLen];
        stringify(nums + (i * length), length, output);
        fflush(stdout);
        write(sockfds[i], output, outLen);

        struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&cli_addr;
        struct in_addr ipAddr = pV4Addr->sin_addr;
        char str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &ipAddr, str, INET_ADDRSTRLEN);

        printf("\nGot request from %s", str);

        printf("\nSending '%d' numbers to socket '%d'", length, sockfds[i]);
        fflush(stdout);
    }
    for (i = 0; i < splitLevel; ++i) {
        int outLen = chars(nums + (i * length), length);
        char input[outLen];
        read(sockfds[i], input, outLen);
        printf("\nRead from socket: '%d'\n%s", sockfds[i], input);
        fflush(stdout);
        saveToArray(input, length, nums + (i * length));
        close(sockfds[i]);
    }

    myRecMerge(nums, size, splitLevel, 0);

    printf("\nFinal answer: ");
    for (i = 0; i < size - 1; ++i)
        printf("%d,", nums[i]);

    printf("%d\n", nums[size - 1]);
    close(sockfd);
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
int comparator(const void* a, const void* b) {
    int num1 = *(const int *)a;
    int num2 = *(const int *)b;
    if (num1 > num2)
        return 1;
    if (num1 == num2)
        return 0;
    return -1;
}

void error(char *msg) {
    perror(msg);
    exit(1);
}
int digits(int num) {
    if (num <= 9)
        return 1;

    int n = 0;
    while (num != 0) {
        n++;
        num /= 10;
    }
    return n;
}
void stringify(int *nums, int size, char* output) {
    char buff[10];
    int k = 0;
    for (int i = 0; i < size; ++i) {
        sprintf(buff, "%d", nums[i]);
        int d = digits(nums[i]);
        for (int j = 0; j < d; ++j, k++)
            output[k] = buff[j];
        output[k++] = ',';
    }
    output[k - 1] = '\0';
}
int chars(int* nums, int size) {
    int cs = 0;
    for (int i = 0; i < size; ++i)
        cs += digits(nums[i]) + 1;
    return cs;
}
void saveToArray(const char *string, int size, int *nums) {
    for (int i = 0, currentSize = 0; i < size; ++i) {
        sscanf(string + currentSize, "%d,", &nums[i]);
        currentSize += digits(nums[i]) + 1;
    }
}