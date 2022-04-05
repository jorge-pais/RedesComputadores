#ifndef RECEIVER_
#define RECEIVER_

#include "utils.h"

/* 
Globally declared serial terminal file descriptor
*/
static int rx_fd;

/* 
Open logical connection on the receiver end
*/
int receiver_llopen(linkLayer connectionParameters){

    rx_fd = configureSerialterminal(connectionParameters);

    unsigned char cmdSet[] = {FLAG, A_tx, C_SET, A_tx ^ C_UA, FLAG};

    if(getCommand(rx_fd, cmdSet, 3) < 0){
        perror("Haven't received SET");
        return -1;
    }
    printf("Received SET, sending UA\n");

    unsigned char cmdUA[] = {FLAG, A_tx, C_UA, A_tx ^ C_UA, FLAG};

    if(write(rx_fd, cmdUA, 5) < 0){
        perror("Error writing to serial port");
        return -1;
    }

    return 0;
}

int receiver_llclose(){

    return 0;
}

#endif