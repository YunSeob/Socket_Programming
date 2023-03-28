# Linux
# gcc server3.c -o server
gcc server_rtsp.c -o server_rtsp
./server_rtsp 10.125.34.164 9554

# gcc -o server server3.c 'pkg-config --cflags --libs gstreamer-1.0'