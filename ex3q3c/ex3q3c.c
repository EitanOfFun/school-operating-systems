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
void myMergeSort(int *nums, size_t size, int startIndex);
void myMerge(int *nums, size_t size);

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;

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
    bcopy(server->h_addr, (char *)  &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sockfd,(const struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    write(sockfd, "", 1);

    int nums[MSG_LEN];
    ssize_t rc = read(sockfd, nums, MSG_LEN);
    size_t length = rc / sizeof(int);

    printf("\ngot from server: \n%d", nums[0]);
    for (int j = 1; j < length; ++j) {
        printf(",%d", nums[j]);
    }
    myMergeSort(nums, length, 0);

    printf("\nsorted: %d", nums[0]);
    for (int j = 1; j < length; ++j) {
        printf(",%d", nums[j]);
    }
    printf("\n");
    write(sockfd, nums, (size_t) rc);

    close(sockfd);
    return 0;
}
void myMergeSort(int *nums, size_t size, int startIndex) {
    if (size == 1)
        return;
    else {
        myMergeSort(nums, size / 2, startIndex);
        myMergeSort(nums, size / 2, startIndex + (size / 2));
        myMerge(nums + startIndex, size);
    }
}
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