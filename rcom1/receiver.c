#include "receiver.h"

/* 
Globally declared serial terminal file descriptor
*/
static int rx_fd;
//u_int8_t timeoutFlag, timerFlag;

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

/* int receiver_llclose(linkLayer connectionParameters){
    // DISC frame header
    unsigned char cmdDisc[] = {FLAG, A_tx, C_DISC, A_tx ^ C_DISC, FLAG};
    // UA frame header expected to receive
    unsigned char cmdUA[] = {FLAG, A_rx, C_UA, A_rx ^ C_UA, FLAG};

    int timeoutCount = 0;

    if(getCommand(rx_fd, cmdDisc, 3) < 0){
        perror("Haven't received DISC command");
        return -1;
    }
    printf("Received DISC, sending DISC back\n");

    (void) signal(SIGALRM, timeOut);
    
    int res = write(rx_fd, cmdDisc, 5);
    if(res < 0){
        perror("Error writing to serial port");
        return -1;
    }
    printf("DISC sent back\n", res);

    timeoutFlag = 0, timeoutCount = 0, timerFlag = 1;

    while (timeoutCount < connectionParameters.timeOut){
        if(timerFlag){
            alarm(3);
            timerFlag = 0;
        }

        int readResult = getCommand(rx_fd, cmdUA, 5);

        if(readResult < 0){
            perror("Error reading command from serial port");
            return -1;
        }
        else if(readResult > 0){ //Success
            signal(SIGALRM, SIG_IGN); //disable interrupt handler
            printf("Received UA, connection established\n");
            return 1;
        }

        if(timeoutFlag){
            int res = write(rx_fd, cmdDisc, 5);
            if(res < 0){
                perror("Error writing to serial port");
                return -1;
            }
            printf("DISC sent back again", res);
            timeoutCount++;
        }
    }    

    return -1;
} */