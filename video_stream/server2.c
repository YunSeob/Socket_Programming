#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_BUFF_SIZE 4096

int main() {
    char *ip_address = "10.125.34.164";
    char *my_ip = "10.125.34.158";
    int port = 9554;
    char *username = "admin";
    char *password = "Mobile306!";
    char *rtsp_path = "/profile2/media.smp";
    char rtsp_request[MAX_BUFF_SIZE] = {0};
    char response[MAX_BUFF_SIZE] = {0};
    int sock_fd, bytes_received, total_packets = 0;
    struct sockaddr_in server_addr;
    
    // create socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // set server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_address);
    // server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    // connect to server
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }

    // construct rtsp request
    sprintf(rtsp_request, "DESCRIBE rtsp://%s:%s@%s:%d%s RTSP/1.0\r\n"
                           "CSeq: 1\r\n"
                           "User-Agent: VLC/2.2.4 LibVLC/2.2.4\r\n"
                           "Accept: application/sdp\r\n"
                           "\r\n", username, password, ip_address, port, rtsp_path);

    // send rtsp request to server
    if (send(sock_fd, rtsp_request, strlen(rtsp_request), 0) < 0) {
        perror("send failed");
        exit(EXIT_FAILURE);
    }

    // receive rtsp response from server
    while ((bytes_received = recv(sock_fd, response, MAX_BUFF_SIZE, 0)) > 0) {
        total_packets++;
        printf("aaa");
        memset(response, 0, sizeof(response));
    }

    printf("Total packets received: %d\n", total_packets);

    close(sock_fd);
    return 0;
}
