#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8554
#define MAX_BUFFER_SIZE 2048

void error_handling(char* message);

int main(int argc, char* argv[]){
    int servSocket, clntSocket, bytesRead;
    char buffer[MAX_BUFFER_SIZE];
    struct sockaddr_in serv_addr, clnt_addr;

    if((servSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        error_handling("socket() Error");
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(servSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
        error_handling("bind() error");
    }

    if(listen(servSocket, 1) < 0){
        error_handling("listen() error");
    }

    printf("RTSP Server Listening on port %d ...", PORT);

    while(1){
        socklen_t clnt_addr_size = sizeof(clnt_addr);

        if((clntSocket = accept(servSocket, (struct sockaddr*)&clnt_addr, &clnt_addr_size)) < 0){
            error_handling("accept() error");
        }
        printf("Accepted Client Connection from %s:%d\n", inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));

        if((bytesRead = read(clntSocket, buffer, MAX_BUFFER_SIZE)) < 0){
            error_handling("read() error");
        }

        char *response = "RTSP/1.0 200 OK\r\nContent-Length : 0\r\n\r\n";
        if(write(clntSocket, response, strlen(response)) < 0){
            error_handling("Error Sending Response");
            close(clntSocket);
        }
        close(clntSocket);
    }
    close(servSocket);

    return 0;
}

void error_handling(char *message){
    fputs(message,stderr);
    fputc('\n', stderr);
    exit(1);
}