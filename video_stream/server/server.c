#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 4096

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

int main(int argc, char *argv[]){
    if(argc != 3){
        fprintf(stderr, "Usage %s <server> <port>\n", argv[0]);
        exit(1);
    }

    char *camera_ip = argv[1];
    int camera_port = atoi(argv[2]);
    char send_msg[1024], recv_msg[4096], buffer[BUFFER_SIZE];

    int camera_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(camera_sock < 0){
        perror("[ERROR] : IP CAMEARA SOCKET");
        exit(1);
    }

    // Connect to IP Camera Socket
    struct sockaddr_in camera_addr;
    memset(&camera_addr, 0, sizeof(camera_addr));
    camera_addr.sin_family = AF_INET;
    camera_addr.sin_port = htons(camera_port);
    camera_addr.sin_addr.s_addr = inet_addr(camera_ip);
    if(connect(camera_sock, (struct sockaddr*)&camera_addr, sizeof(camera_addr)) < 0){
        perror("[ERROR] : IP CAMEARA CONNECTION");
        exit(1);
    }
    char *rtsp_address = "rtsp://10.125.34.164:9554/profile2/media.smp";
    int CSeq = 2;
    // [RTSP] OPTIONS
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
    int serv_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(serv_sock < 0){
        perror("[ERROR] SERVER SOCKET ERROR");
        exit(1);
    }
    struct sockaddr_in serv_addr;
    socklen_t serv_addr_size;
    int serv_port = atoi("40160");          // client_port : Receive Video stream PORT
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serv_port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr_size = sizeof(serv_addr);

    if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
        perror("[ERROR] SERVER BIND ERROR");
        exit(1);
    }
    memset(&buffer, 0, sizeof(buffer));
    while(count < 10){
        bytes = recvfrom(serv_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&serv_addr, &serv_addr_size);
        printf("[BYTES]%d\n[BUFFER]\n%s\n",bytes, buffer);
        count++;
    }
    
    close(camera_sock);

    return 0;
}