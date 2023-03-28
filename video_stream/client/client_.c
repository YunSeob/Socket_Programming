#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define BUFFER_SIZE 4096
// gcc -o client.exe client.c -lws2_32 -Wall

void error_handling(char *message);
void send_receive(int camera_sock, char* send_msg, char*recv_msg, char *message);
int get_session(char* recv_msg);


int main(int argc, char *argv[]){
    if(argc != 3){
        fprintf(stderr, "Usage %s <server> <port>\n", argv[0]);
        exit(1);
    }
    WSADATA wsadata;
    SOCKET camera_sock;
    char *camera_ip = argv[1];
    int camera_port = atoi(argv[2]);
    char send_msg[1024], recv_msg[4096], buffer[BUFFER_SIZE];
    struct sockaddr_in camera_addr;

    if(WSAStartup(MAKEWORD(2,2), &wsadata) != 0){
        error_handling("[ERROR] WSAStartup ERROR");
    }
    camera_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(camera_sock == INVALID_SOCKET){
        error_handling("[ERROR] SOCKET ERROR");
        WSACleanup();
        return 1;
    }
    memset(&camera_addr, 0, sizeof(camera_addr));
    camera_addr.sin_family = AF_INET;
    camera_addr.sin_port = htons(camera_port);
    camera_addr.sin_addr.s_addr = inet_addr(camera_ip);
    if(connect(camera_sock, (struct sockaddr*)&camera_addr, sizeof(camera_addr)) == SOCKET_ERROR){
        error_handling("[ERROR] CONNECTION ERROR");
        WSACleanup();
        return 1;
    }

    char *rtsp_address = "rtsp://10.125.34.164:9554/profile2/media.smp";
    int CSeq = 2;

    snprintf(send_msg, sizeof(send_msg), "OPTIONS %s RTSP/1.0\r\n"
                                         "CSeq: %d\r\n"
                                         "User-Agent: ys\r\n\r\n", rtsp_address, CSeq++);
    send_receive(camera_sock, send_msg, recv_msg, "OPTIONS");

    // [RTSP] DESCRIBE
    snprintf(send_msg, sizeof(send_msg), "DESCRIBE %s RTSP/1.0\r\n"
                                         "CSeq: %d\r\n"
                                         "User-Agent: ys\r\n"
                                         "Accept: application/sdp\r\n\r\n", rtsp_address, CSeq++);
    send_receive(camera_sock, send_msg, recv_msg, "DESCRIBE");

    // [RTSP] SETUP
    snprintf(send_msg, sizeof(send_msg), "SETUP %s/trackID=v RTSP/1.0\r\n"
                                         "CSeq: %d\r\n"
                                         "User-Agent: ys\r\n"
                                         "Transport: RTP/AVP;unicast;client_port=40160-40161\r\n\r\n", rtsp_address, CSeq++);
    // memset(&send_msg, 0, sizeof(send_msg));
    memset(&recv_msg, 0, sizeof(recv_msg));
    send_receive(camera_sock, send_msg, recv_msg, "SETUP");

    // [RTSP] PLAY
    // memset(&send_msg, 0, sizeof(send_msg));
    // memset(&recv_msg, 0, sizeof(recv_msg));
    snprintf(send_msg, sizeof(send_msg), "PLAY %s RTSP/1.0\r\n"
                                         "CSeq: %d\r\n"
                                         "User-Agent: ys\r\n"
                                         "Session: %d\r\n"
                                         "Range: npt=0.000-\r\nTimeout: 60\r\n\r\n", rtsp_address, CSeq++, get_session(recv_msg));
    send_receive(camera_sock, send_msg, recv_msg, "PLAY");

    int bytes, count = 0;
    // Create multicast socket
    SOCKET send_sock;
    int time_live = 64;
    struct sockaddr_in send_addr;
    int send_port = 9900;

    send_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(send_sock == INVALID_SOCKET){
        perror("[ERROR] SEND SOCKET ERROR");
    }
    setsockopt(send_sock, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&time_live, sizeof(time_live));

    // Set multicast group
    memset(&send_addr, 0, sizeof(send_addr));
    send_addr.sin_family = AF_INET;
    send_addr.sin_port = htons(send_port);
    send_addr.sin_addr.s_addr = inet_addr("224.1.1.1");
    
    SOCKET client_sock;
    struct sockaddr_in client_addr;
    int client_addr_size;
    int client_port = atoi("40160");

    client_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(client_sock == INVALID_SOCKET)
        perror("[ERROR] CLIENT SOCKET ERROR");

    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(client_port);
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    client_addr_size = sizeof(client_addr);

    if(bind(client_sock, (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0){
        error_handling("[ERROR] BIND ERROR");
    }
    
    memset(&buffer, 0, sizeof(buffer));
    while(count < 20){
        bytes = recvfrom(client_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_addr_size);
        sendto(send_sock, buffer, strlen(buffer), 0, (struct sockaddr*)&send_addr, sizeof(send_addr));
        printf("[BYTES]%d\t[COUNT]%d\n[BUFFER]%s\n",bytes, count, buffer);
        count++;
    }
    
    // Close the client socket
    closesocket(camera_sock);
    closesocket(send_sock);
    WSACleanup();
    return 0;
}

void error_handling(char *message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
void send_receive(int camera_sock, char* send_msg, char*recv_msg, char *message){
    send(camera_sock, send_msg, strlen(send_msg), 0);
    printf("[SEND %s]\n%s\n", message, send_msg);    
    recv(camera_sock, recv_msg, BUFFER_SIZE, 0);
    printf("[RECEIVE %s]\n%s\n", message, recv_msg);
}
int get_session(char* recv_msg){
    char *ptr = strstr(recv_msg, "Session: ");
    char session_[10];
    int j = 0;
    while(*ptr != ';'){
        if(*ptr != 'S' && *ptr != 'e' && *ptr !='s' && *ptr != 'i' && *ptr !='o' && *ptr != 'n' && *ptr != ':' && *ptr != ' '){
        session_[j++] = *ptr;
      }
      ptr++;
    }
    char *arr = (char*)malloc(sizeof(char)*(j+1));
    for(int i = 0; i < j; i++){
        arr[i] = session_[i];
    }
    arr[j] = '\0';
    // printf("arr : %s\n", arr);

    int result = 0;
    while(*arr){
        result = result*10 +(*arr-'0');
        arr++;
    }
    // free(arr);
    // printf("RESULT : %d\n", result);
    return result;
}