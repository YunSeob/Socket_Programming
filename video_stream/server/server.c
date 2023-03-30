#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

int main(int argc, char *argv[]){

    int bytes, count = 0;
    char buffer[BUFFER_SIZE];
    struct ip_mreq mreq;
    struct sockaddr_in serv_addr;
    int serv_port = 9900;
    socklen_t serv_addr_size;

    int serv_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(serv_sock < 0){
        perror("[ERROR] SERVER SOCKET ERROR");
        exit(1);
    }
    // set reuse option to avoid "address already in use" error
    int optval = 1;
    if(setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0){
        perror("[ERROR] SETSOCK ERROR");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serv_port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr_size = sizeof(serv_addr);
    
    if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
        perror("[ERROR] SERVER BIND ERROR");
        exit(1);
    }
    
    // join multicast group
    mreq.imr_multiaddr.s_addr = inet_addr("224.1.1.1");
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if(setsockopt(serv_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0){
        perror("[ERROR] SETSOCKOPT ERROR");
        exit(1);
    }
    memset(&buffer, 0, sizeof(buffer));
    // FILE *fp = fopen("udp.txt", "wb");
    while(count < 50){
        bytes = recvfrom(serv_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&serv_addr, &serv_addr_size);
        printf("[COUNT]%d\t[BYTES]%d\n", count, bytes);

        // save raw data to txt file
        // fwrite(buffer, sizeof(char), strlen(buffer), fp);
        count++;
    }
    // fclose(fp);
    close(serv_sock);

    return 0;
}