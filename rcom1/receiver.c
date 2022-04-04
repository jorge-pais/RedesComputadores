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

    int fd = configureSerialterminal(connectionParameters);

    unsigned char cmdSet[] = {FLAG, A_tx, C_SET};

    getCommand(fd, cmdSet, 3);

    return 0;
}

int receiver_llclose();

#endif