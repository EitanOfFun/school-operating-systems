#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MSG_LEN 255

void error(char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[MSG_LEN];
    char rbuf[MSG_LEN];

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

    while(1){

        scanf("%s",buffer);
        if (strcmp("exit", buffer) == 0)
            break;

        rc = write(sockfd, buffer, strlen(buffer)+1) ;
        rc = read(sockfd, rbuf,  MSG_LEN+1) ;
        if (strcmp("exit", rbuf) == 0)
            break;
        printf("server: %s\n", rbuf);
    }

    close(sockfd);
    return 0;
}
