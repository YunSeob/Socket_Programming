#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
void error_handling(char *message);

int main(int argc, char *argv[]){
    WSADATA wsadata;
    SOCKET hSocket;
    SOCKADDR_IN serv_addr;

    char message[30];
    int strlen = 0;
    int idx =0, readlen = 0;

    if(argc != 3){
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    if(WSAStartup(MAKEWORD(2,2), &wsadata) != 0){
        error_handling("WSAStartup() error");
    }

    hSocket = socket(PF_INET, SOCK_STREAM, 0);
    if(hSocket == INVALID_SOCKET){
        error_handling("socket() error");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if(connect(hSocket, (SOCKADDR*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR){
        error_handling("connect() error");
    }

    while(readlen = recv(hSocket, &message[idx++], 1, 0)){
        if(readlen == -1)
            error_handling("recv() error");
        strlen += readlen;
    }
    printf("Message from Server : %s\n", message);
    printf("Function read call count : %d \n", strlen);

    closesocket(hSocket);
    WSACleanup();

    return 0;
}

void error_handling(char *message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}