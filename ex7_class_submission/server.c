#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MSG_LEN 255

void error(char *msg) {
    perror(msg);
    exit(1);
}
int main(int argc, char *argv[])
{
    int sockfd, newsockfd;
    char buffer[MSG_LEN];
    char rbuf[MSG_LEN];
    struct sockaddr_in serv_addr;
    struct sockaddr cli_addr;
    socklen_t clilen;
    int n,rc;

    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if (sockfd < 0)
        error("ERROR opening socket");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(argv[1]));


    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    listen(sockfd,5);

    newsockfd = accept(sockfd, &cli_addr, &clilen);
    if (newsockfd < 0)
        perror("ERROR on accept");

    while (1) {

        rc = read(newsockfd, rbuf, MSG_LEN);
        if (rc == 0)
            break;
        else if (rc > 0) {
            if (strcmp(rbuf, "exit") == 0)
                break;
            printf("client: %s\n", rbuf);
            scanf("%s", buffer);
            write(newsockfd, buffer, strlen(buffer) + 1);

        } else {
            perror("read() failed") ;
            exit(EXIT_FAILURE) ;
        }
    }

    close(newsockfd);

    close(sockfd);  //or goto accept to wait for another clients
    return 0;
}
