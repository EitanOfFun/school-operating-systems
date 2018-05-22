#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MSG_LEN 255


void error(char *msg);
int digits(int num);
void stringify(int *nums, int size, char* output);
int chars(int* nums, int size);
void saveToArray(const char *string, int size, int *nums);

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[MSG_LEN];

    if (argc < 3) {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        exit(0);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)  &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sockfd,(const struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");


    write(sockfd, "", 1);

    read(sockfd, buffer, MSG_LEN+1);

    int length = len(buffer);
    int nums[length];
    saveToArray(buffer, length, nums);
    mergesort(nums, (size_t) length, sizeof(int), comparator);


    printf("\ngot from server: %s\n", buffer);
    printf("\nsorted: ");
    for (int i = 0; i < length; ++i)
        printf("%d,", nums[i]);

    printf("\n");
    char output[chars(nums, length)];
    stringify(nums, length, output);
    write(sockfd, output, strlen(output) + 1);

    close(sockfd);
    return 0;
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
int len(const char* numsS){
    int commas = 0;
    for (int i = 0; numsS[i] != '\0'; ++i)
        if (numsS[i] == ',')
            commas++;
    return commas + 1;
}
int digits(int num) {
    if (num <= 9)
        return 1;
    int n;
    for (n = 0; num != 0; ++n)
        num /= 10;
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
