#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFSIZE 65536

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server> <port>\n", argv[0]);
        exit(1);
    }

    char *server = argv[1];
    int port = atoi(argv[2]);
    char sendline[1024], recvline[4096];

    // Create RTSP socket
    int rtsp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // int rtsp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (rtsp_sockfd < 0) {
        perror("Error creating RTSP socket");
        exit(1);
    }

    // Connect to server
    struct sockaddr_in rtsp_addr;
    memset(&rtsp_addr, 0, sizeof(rtsp_addr));
    rtsp_addr.sin_family = AF_INET;
    rtsp_addr.sin_addr.s_addr = inet_addr(server);
    rtsp_addr.sin_port = htons(port);
    if (connect(rtsp_sockfd, (struct sockaddr*)&rtsp_addr, sizeof(rtsp_addr)) < 0) {
        perror("Error connecting to server");
        exit(1);
    }

    // Send RTSP OPTIONS request
    snprintf(sendline, sizeof(sendline), "OPTIONS rtsp://10.125.34.164:9554/profile2/media.smp RTSP/1.0\r\nCSeq: 2\r\nUser-Agent: ys\r\n\r\n");
    write(rtsp_sockfd, sendline, strlen(sendline));
  // Receive RTSP response
    read(rtsp_sockfd, recvline, 4096);
    printf("%s", recvline);
    // Send DESCRIBE
    snprintf(sendline, sizeof(sendline), "DESCRIBE rtsp://10.125.34.164:9554/profile2/media.smp RTSP/1.0\r\nCSeq: 3\r\nUser-Agent: ys\r\nAccept: application/sdp\r\n\r\n");
    write(rtsp_sockfd, sendline, strlen(sendline));

    read(rtsp_sockfd, recvline, 4096);
    printf("%s", recvline);

    snprintf(sendline, sizeof(sendline), "SETUP rtsp://10.125.34.164:9554/profile2/media.smp/trackID=v RTSP/1.0\r\nCSeq: 4\r\nUser-Agent: ys\r\nTransport: RTP/AVP;unicast;client_port=40180-40181\r\n\r\n");
    write(rtsp_sockfd, sendline, strlen(sendline));

    read(rtsp_sockfd, recvline, 4096);
    printf("%s", recvline);
    
    char *ptr = strstr(recvline, "Session: ");
    // sleep(1);
    char session_[10];
    int j = 0;
    while(*ptr != ';'){
      // printf("%c\n", *ptr);
      if(*ptr != 'S' && *ptr != 'e' && *ptr !='s' && *ptr != 'i' && *ptr !='o' && *ptr != 'n' && *ptr != ':' && *ptr != ' '){
        session_[j++] = *ptr;
      }
      ptr++;
    }
    char* arr = (char*)malloc(sizeof(char)*(j+1));
    for(int i = 0; i < j; i++){
      arr[i] = session_[i];
    }
    arr[j] = '\0';
    printf("arr : %s\n", arr);

    snprintf(sendline, sizeof(sendline), "PLAY rtsp://10.125.34.164:9554/profile2/media.smp RTSP/1.0\r\nCSeq: 5\r\nUser-Agent: ys\r\nSession: %s\r\nRange: npt=0.000-\r\n\r\n",arr);
    write(rtsp_sockfd, sendline, strlen(sendline));

    read(rtsp_sockfd, recvline, 4096);
    printf("%s", recvline);
    
    free(arr);

    int rtp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (rtp_sockfd < 0) {
        perror("Error creating RTP socket");
        exit(1);
    }

    // Bind socket to RTP ports
    struct sockaddr_in rtp_addr;
    int rtp_port = atoi(argv[2]);
    memset(&rtp_addr, 0, sizeof(rtp_addr));
    rtp_addr.sin_family = AF_INET;
    rtp_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    rtp_addr.sin_port = htons(rtp_port);
    if (bind(rtp_sockfd, (struct sockaddr*)&rtp_addr, sizeof(rtp_addr)) < 0) {
        perror("Error binding RTP socket");
        exit(1);
    }
    while (1) {
        read(rtp_sockfd, recvline, 4096);
        printf("%s", recvline);
    }


    // snprintf(sendline, sizeof(sendline), "PLAY rtsp://10.125.34.164:9554/0/profile2/media.smp RTSP/1.0\r\nCSeq: 3\r\nUser-Agent: Samsung techwin rtp/rtsp client\r\nTransport: RTP/AVP/TCP;unicast;interleaved=2-3\r\nSession: 50589\r\n\r\n");
    // write(rtsp_sockfd, sendline, strlen(sendline));

    // read(rtsp_sockfd, recvline, 4096);
    // printf("%s", recvline);

    // Send SETUP
    // snprintf(sendline, sizeof(sendline), "SETUP rtsp://10.125.34.164:9554/0/profile2/media.smp/trackID=v RTSP/1.0\r\nCSeq: 3\r\nUser-Agent: Samsung techwin rtp/rtsp client\r\nTransport: RTP/AVP/TCP;unicast;interleaved=0-1\r\n\r\n");
    // write(rtsp_sockfd, sendline, strlen(sendline));

    // read(rtsp_sockfd, recvline, 4096);
    // printf("%s", recvline);

    // snprintf(sendline, sizeof(sendline), "SETUP rtsp://10.125.34.164:9554/0/profile2/media.smp/trackID=m RTSP/1.0\r\nCSeq: 4\r\nUser-Agent: Samsung techwin rtp/rtsp client\r\nTransport: RTP/AVP/TCP;unicast;interleaved=2-3\r\nSession: 50589\r\n\r\n");
    // write(rtsp_sockfd, sendline, strlen(sendline));

    // read(rtsp_sockfd, recvline, 4096);
    // printf("%s", recvline);

    // snprintf(sendline, sizeof(sendline), "PLAY rtsp://10.125.34.164:9554/0/profile2/media.smp RTSP/1.0\r\nCSeq: 7\r\nUser-Agent: Samsung techwin rtp/rtsp client\r\nTransport: RTP/AVP/TCP;unicast;interleaved=2-3\r\nSession: 50589\r\n\r\n");
    // write(rtsp_sockfd, sendline, strlen(sendline));

    // read(rtsp_sockfd, recvline, 4096);
    // printf("%s", recvline);

    // Parse SDP to obtain RTP port numbers
    // ...

    //Create UDP socket for RTP receiver
    // int rtp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    // if (rtp_sockfd < 0) {
    //     perror("Error creating RTP socket");
    //     exit(1);
    // }

    // // Bind socket to RTP ports
    // struct sockaddr_in rtp_addr;
    // int rtp_port = atoi(argv[2]);
    // memset(&rtp_addr, 0, sizeof(rtp_addr));
    // rtp_addr.sin_family = AF_INET;
    // rtp_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // rtp_addr.sin_port = htons(rtp_port);
    // if (bind(rtp_sockfd, (struct sockaddr*)&rtp_addr, sizeof(rtp_addr)) < 0) {
    //     perror("Error binding RTP socket");
    //     exit(1);
    // }

    // // Listen for incoming RTP packets

    // Close sockets
    
    close(rtsp_sockfd);
    // close(rtp_sockfd);

    return 0;
}
