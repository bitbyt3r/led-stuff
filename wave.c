/* 
 * udpclient.c - A simple UDP client
 * usage: udpclient <host> <port>
 */
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include "portaudio.h"

#define BUFSIZE 65536
#define VOLUME 250
#define FPS 30
#define FRAMECOUNT 44100 / FPS

/* 
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}

int sockfd, port, n, msglen;
int serverlen;
struct sockaddr_in serveraddr;
struct hostent *server;
char *host;
char buf[BUFSIZE];
static volatile int running;

void intHandler(int dummy) {
  running = 0;
}

int clear() {
    sprintf(buf, "P6\n512 32\n255\n");
    msglen = 14;
    for (int y=0;y<32;y++) {
      for (int x=0;x<512;x++) {
        buf[3*(512*y+x)+14] = 0;
        buf[3*(512*y+x)+15] = 0;
        buf[3*(512*y+x)+16] = 0;
        msglen = msglen + 3;
      }
    }

    /* send the message to the server */
    n = sendto(sockfd, buf, msglen, 0, &serveraddr, serverlen);
    if (n < 0) 
      error("ERROR in sendto");
    return 0;
}
int display(const float *input,
            void *output,
            unsigned long frameCount,
            const PaStreamCallbackTimeInfo* timeInfo,
            PaStreamCallbackFlags statusFlags,
            void *userData
           ) {
    sprintf(buf, "P6\n512 32\n255\n");
    msglen = 14;
    int color = 1;
    for (int y=0;y<32;y++) {
      for (int x=0;x<512;x++) {
        color = 1;
        if (y <= 16) {
          if (input[x]*VOLUME+16 <= y) {
            color = 255;
          }
        } else {
          if (y <= input[x]*VOLUME+16) {
            color = 255;
          }
        }
        buf[3*(512*y+x)+14] = color;
        buf[3*(512*y+x)+15] = color;
        buf[3*(512*y+x)+16] = color;
        msglen = msglen + 3;
      }
    }

    /* send the message to the server */
    n = sendto(sockfd, buf, msglen, 0, &serveraddr, serverlen);
    if (n < 0) 
      error("ERROR in sendto");
    return 0;
}

int main(int argc, char **argv) {
    running = 1;
    signal(SIGINT, intHandler);
    
    host = "10.1.253.237";
    port = 1337;

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(host);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", host);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(port);
    serverlen = sizeof(serveraddr);

    PaError err;
    err = Pa_Initialize();
    if( err != paNoError )
        printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );


    PaStream *stream;
    static int data;
    err = Pa_OpenDefaultStream( &stream,
                                1,
                                0,
                                paFloat32,
                                44100,
                                FRAMECOUNT,
                                display,
                                &data );
    if( err != paNoError )
        printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );    

    err = Pa_StartStream( stream );
    if( err != paNoError )
        printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );    
    
    while (running) {
    }
    
    err = Pa_StopStream( stream );
    if( err != paNoError )
        printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );    
        
    err = Pa_Terminate();
    if( err != paNoError )
        printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );    

    clear();
    return 0;
}
