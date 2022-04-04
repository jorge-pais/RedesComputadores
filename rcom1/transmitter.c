#ifndef TRANSMITTER_
#define TRANSMITTER_

#include "utils.h"

//static struct termios oldtio, newtio;
static int tx_fd;

int transmitter_llopen(linkLayer connectionParameters){

    tx_fd = configureSerialterminal(connectionParameters);

    unsigned char cmdSet[] = {FLAG, A_tx, C_SET, A_tx ^ C_SET, FLAG};
    
    //Fazer isto num ciclo para verificar o timeout
    if(write(tx_fd, cmdSet, 5 < 0)){
        perror("Error writing to serial port");
        return -1;
    }

    unsigned char cmdUA[] = {FLAG, A_tx, C_UA, A_tx ^ C_UA, FLAG};
    if(getCommand(tx_fd, cmdUA, 5) < 0){
        //do something
        return -1;
    }

    return 0;
}
#endif