#include "server.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>
#include "../NDSSystem.h"

#define PORT 27015

int frame_count;
int o_port;
bool running;
int socketfd, new_socket;
struct sockaddr_in address; 
int addrlen = sizeof(address); 
std::thread* acceptor;
int opt = 1; 

char buffer[1024] = {0}; 

unsigned short key_state = 0;
bool touched = false;
int x = 0;
int y = 0;

void acceptAndListen() {
    while(running) {
        printf("Accepting\n");
        if ((new_socket = accept(socketfd, (struct sockaddr *)&address,  
                        (socklen_t*)&addrlen))<0) 
        { 
            perror("accept"); 
            Init(o_port);
            continue;
            //exit(EXIT_FAILURE); 
        } 
        
        printf("Accepted\n");
        key_state = 0;
        int b = 0;
        while(true) {
            int r = read( new_socket , buffer, 1024);
            printf("r: %d\n", r);
            if (r == 0 || r == -1) {
                break;
            }
            b += r;
            printf("Read\n");
            if (b >= 4) {
                char type = buffer[0];
                if (type == 0) {
                    //HANDLE TOUCH
                    char value = buffer[1];
                    printf("type:%d,value:%d\n", type, value);
                    if (value > 0) {
                        int x = buffer[2];
                        int y = buffer[3];
                        printf("x:%d,y:%d\n", x, y);
                        NDS_setTouchPos(x, y);
                    } else {
                        NDS_releaseTouch();
                    }
                } else {
                    //HANDLE BUTTON
                    char value = buffer[1];
                    printf("type:%d,value:%d\n", type, value);
                    if (value > 0) {
                        key_state |= (1 << (type-1)); 
                    } else {
                        key_state &=  0xFF ^ (1 << (type-1));
                    }
                    printf("state:%02x\n", key_state);
                }
                b -= 4;
                for (int i = 0;  i < b; i++) {
                    buffer[i] = buffer[i+4];
                }
            }
        }
        printf("Close\n");
        close(new_socket);
    }
}

void Init(int port) { 
    frame_count = 0;
    o_port = port;   
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
    printf("port:%d\n", port);
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( port ); 

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

int slot = 0;
int Run(unsigned short* keys) {
    (*keys) = key_state;

    if (frame_count == 60 * 60 * 30) {
        frame_count = 0;
        slot = (slot % 6) + 1;
        return slot;
    }
    frame_count ++;

    return 0;
}

void Destroy() {
    running = false;
    acceptor->join();
}
