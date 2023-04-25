#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#define BUFFER_SIZE 4096
// gcc -o client.exe client.c -lws2_32 -Wall

void error_handling(char *message);
void send_receive(int camera_sock, char* send_msg, char*recv_msg, char *message);
int get_session(char* recv_msg);
char *get_rtsp_address(char *a, char *b, char *c, char *d, char *e);


int main(int argc, char *argv[]){
    if(argc != 3){
        fprintf(stderr, "Usage %s <server> <port>\n", argv[0]);
        exit(1);
    }

    // Camera Socket
    int camera_sock;
    char *camera_ip = argv[1];
    int camera_port = atoi(argv[2]);
    char send_msg[1024], recv_msg[4096], buffer[BUFFER_SIZE];
    struct sockaddr_in camera_addr;

    camera_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(camera_sock < 0){
        error_handling("[ERROR] SOCKET ERROR");
        return 1;
    }
    memset(&camera_addr, 0, sizeof(camera_addr));
    camera_addr.sin_family = AF_INET;
    camera_addr.sin_port = htons(camera_port);
    camera_addr.sin_addr.s_addr = inet_addr(camera_ip);
    if(connect(camera_sock, (struct sockaddr*)&camera_addr, sizeof(camera_addr)) < 0){
        error_handling("[ERROR] CONNECTION ERROR");
        return 1;
    }
    
    //char *rtsp_address = "rtsp://192.168.1.10:9554/profile2/media.smp";
    char rtsp_address[100] = "rtsp://";
    char *rtsp_ip = argv[1];
    char *rtsp_port = argv[2];
    char *rtsp_end = "/profile2/media.smp";
    get_rtsp_address(rtsp_address, rtsp_ip, ":", rtsp_port, rtsp_end);
    printf("[RTSP ADDRESS] : %s\n", rtsp_address);
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
                                         "Transport: RTP/AVP;unicast;client_port=40120-40121\r\n\r\n", rtsp_address, CSeq++);
    memset(&recv_msg, 0, sizeof(recv_msg));
    send_receive(camera_sock, send_msg, recv_msg, "SETUP");

    // [RTSP] PLAY
    snprintf(send_msg, sizeof(send_msg), "PLAY %s RTSP/1.0\r\n"
                                         "CSeq: %d\r\n"
                                         "User-Agent: ys\r\n"
                                         "Session: %d\r\n"
                                         "Range: npt=0.000-\r\nTimeout: 60\r\n\r\n", rtsp_address, CSeq++, get_session(recv_msg));
    send_receive(camera_sock, send_msg, recv_msg, "PLAY");

    

    int bytes, count = 0;

    // Create multicast socket
    int send_sock;
    int time_live = 64;
    struct sockaddr_in send_addr;
    int send_port = 9900;

    send_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(send_sock < 0){
        perror("[ERROR] SEND SOCKET ERROR");
    }
    setsockopt(send_sock, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&time_live, sizeof(time_live));

    // Set multicast group
    memset(&send_addr, 0, sizeof(send_addr));
    send_addr.sin_family = AF_INET;
    send_addr.sin_port = htons(send_port);
    send_addr.sin_addr.s_addr = inet_addr("224.1.1.1");
    
    int client_sock;
    struct sockaddr_in client_addr;
    int client_addr_size;
    int client_port = atoi("40120");

    client_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(client_sock < 0)
        perror("[ERROR] CLIENT SOCKET ERROR");

    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(client_port);
    // client_addr.sin_port = htons(camera_port);
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    client_addr_size = sizeof(client_addr);

    if(bind(client_sock, (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0){
        error_handling("[ERROR] BIND ERROR");
    }
    
    memset(&buffer, 0, sizeof(buffer));

    // CSV
    FILE *fp = fopen("data_server.csv", "wb");
    FILE *fp_packet = fopen("data_server.dat", "wb");
    fprintf(fp, "idx,bytes,buffer_size,time\n");
    // Time to analyzing bandwidth and latency
    struct timeval tv;
    double begin, end;
    long long total_bytes_sent =0;
    gettimeofday(&tv, NULL);
    begin = (tv.tv_sec)*1000 + (tv.tv_usec) / 1000;
    
    while(count < 30){
        // bytes = recv(client_sock, buffer, BUFFER_SIZE, 0);
        bytes = recvfrom(client_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_addr_size);
        gettimeofday(&tv, NULL);
        
        sendto(send_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&send_addr, sizeof(send_addr));
        total_bytes_sent += strlen(buffer);
        fprintf(fp, "%d,",(count));
        fprintf(fp, "%d,",bytes);
        fprintf(fp, "%ld,", strlen(buffer));
        fprintf(fp,"%ld\n",(tv.tv_sec)*1000+(tv.tv_usec)/1000);
        fwrite(buffer, strlen(buffer), 1, fp_packet);
        printf("[COUNT]%d\t[BYTES]%d\t[BUFFERSIZE]%ld\n",count, bytes, strlen(buffer));
        // sleep(1);
        count++;
    }
    gettimeofday(&tv, NULL);
    end = (tv.tv_sec)*1000 + (tv.tv_usec) / 1000;
    sleep(1);
    snprintf(send_msg, sizeof(send_msg), "TEARDOWN %s RTSP/1.0\r\n"
                                         "CSeq: %d\r\n"
                                         "User-Agent: ys\r\n"
                                         "Session: %d\r\n\r\n", rtsp_address, CSeq++, get_session(recv_msg));
    send_receive(camera_sock, send_msg, recv_msg, "TEARDOWN");

    double elapsed_time = (end - begin) / 1000;
    double bandwidth_usage = total_bytes_sent / elapsed_time;

    printf("Total Bytes Sent : %lld\n", total_bytes_sent);
    printf("Elapsed time : %.4f seconds\n", elapsed_time);
    printf("Bandwidth Usage : %.2f bytes/second\n", bandwidth_usage);

    // Close the client socket
    close(camera_sock);
    close(send_sock);
    close(client_sock);
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
char *get_rtsp_address(char *a, char *b, char *c, char *d, char *e){
	char *p = a;
	while(*a) a++;
	while(*b) *a++ = *b++;
	while(*c) *a++ = *c++;
	while(*d) *a++ = *d++;
	while(*e) *a++ = *e++;
	*a = '\0';
	return p;
}
