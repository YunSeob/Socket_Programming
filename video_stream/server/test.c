#include <stdio.h>
#include <string.h>

char *concat(char *a, char *b, char *c, char *d){
	char *p = a;
	while(*a) a++;
	while(*b) *a++ = *b++;
	while(*c) *a++ = *c++;
	while(*d) *a++ = *d++;
	*a = '\0';
	return p;
}

int main(int argc, char *argv[]){
	char rtsp_header[100] = "rtsp://";
	char *rtsp_ip = argv[1];
	char *rtsp_port = argv[2];
	char *rtsp_end = "/profile2/media.smp";
	concat(rtsp_header, rtsp_ip, rtsp_port, rtsp_end);
	printf("%s\n", rtsp_header);
	return 0;
}

