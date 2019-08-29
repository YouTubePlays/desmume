#include "server.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>
#include "../NDSSystem.h"

#define KEY_NONE		0
#define KEY_A			1
#define KEY_B			2
#define KEY_SELECT		3
#define KEY_START		4
#define KEY_RIGHT		5
#define KEY_LEFT		6
#define KEY_UP			7
#define KEY_DOWN		8
#define KEY_R			9
#define KEY_L			10
#define KEY_X			11
#define KEY_Y			12
#define KEY_DEBUG		13
#define KEY_BOOST		14
#define KEY_LID			15

#define PORT 25018

bool running;
int socketfd, new_socket;
struct sockaddr_in address; 
int addrlen = sizeof(address); 
std::thread* acceptor;
int opt = 1; 

char buffer[1024] = {0}; 

void acceptAndListen() {
    while(running) {
        printf("Accepting\n");
        if ((new_socket = accept(socketfd, (struct sockaddr *)&address,  
                        (socklen_t*)&addrlen))<0) 
        { 
            perror("accept"); 
            exit(EXIT_FAILURE); 
        } 
        
        printf("Accepted\n");
        while(read( new_socket , buffer, 1024)) {
            printf("%s\n",buffer ); 
        }
    }
}

void Init() {    
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd <= 0) {
        perror("socket failed"); 
        exit(EXIT_FAILURE);
    }

    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                                                  &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( PORT ); 

    // Forcefully attaching socket to the port 8080 
    if (bind(socketfd, (struct sockaddr *)&address,  
                                 sizeof(address))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    if (listen(socketfd, 3) < 0) 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 

    running = true;
    acceptor = new std::thread(acceptAndListen);
}

void Run(unsigned short* keys) {
    // //printf("TEST%d\n", b);
    // b += 1;

    // if (b>100) {
    //     b = -100;
    // }
    
    // if (b>0) {
    //     printf("TEST%d\n", 1 << KEY_DOWN);
    //     NDS_setTouchPos(128, 96);
    //     (*keys) |= 128;
    // } else {
    //     printf("TEST%d\n", 0);
    //     NDS_releaseTouch();
    //     (*keys) = 0;
    // }
}

void Destroy() {
    running = false;
    close(new_socket);
    acceptor->join();
}
