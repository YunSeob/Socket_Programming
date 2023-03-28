#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define SERVER_IP "10.125.34.158"
#define SERVER_PORT 8554
#define MAX_BUFFER_SIZE 2048

// gcc -o client.exe client2.c -lws2_32 -Wall 

void error_handling(char *message);

int main(int argc, char *argv[]){
    WSADATA wsadata;
    SOCKET clntSocket;
    char buffer[MAX_BUFFER_SIZE];
    struct sockaddr_in serv_addr;

    if(WSAStartup(MAKEWORD(2,2), &wsadata) != 0){
        error_handling("WSAStartup() error");
    }

    if((clntSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET){
        error_handling("socket() error");
        WSACleanup();
        return 1;
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8554);
    serv_addr.sin_addr.s_addr = inet_addr("10.125.34.158");

    if(connect(clntSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR){
        error_handling("connect() error");
        closesocket(clntSocket);
        WSACleanup();
        return 1;
    }

    printf("Connected to RTSP server at %s:%d\n", SERVER_IP, SERVER_PORT);

    char *request = "OPTIONS rtsp://10.125.34.164:554\r\n";
    if(send(clntSocket, request, strlen(request), 0) == SOCKET_ERROR){
        error_handling("Error Sending()");
        closesocket(clntSocket);
        WSACleanup();
        return 1;
    }

    int bytesRead;
    if((bytesRead = recv(clntSocket, buffer, MAX_BUFFER_SIZE, 0)) == SOCKET_ERROR){
        error_handling("recv() error");
        closesocket(clntSocket);
        WSACleanup();
        return 1;
    }
    buffer[bytesRead] = '\0';
    printf("Response from server:\n%s\n", buffer);

    // Close the client socket
    closesocket(clntSocket);
    WSACleanup();
    return 0;
}

void error_handling(char *message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}