#ifndef TRANSMITTER_
#define TRANSMITTER_

#include "utils.h"

void timeOut();

//static struct termios oldtio, newtio;
static int tx_fd;
u_int8_t timeoutFlag, timerFlag;

int transmitter_llopen(linkLayer connectionParameters){

    tx_fd = configureSerialterminal(connectionParameters);

    printf("%d \n", newtio.c_cc[VTIME]);

    int timeoutCount = 0;

    // SET frame header
    unsigned char cmdSet[] = {FLAG, A_tx, C_SET, A_tx ^ C_SET, FLAG};
    // UA frame header, what we are expecting to receive
    unsigned char cmdUA[] = {FLAG, A_tx, C_UA, A_tx ^ C_UA, FLAG};

    (void) signal(SIGALRM, timeOut);
    
    int res = write(tx_fd, cmdSet, 5);
    if(res < 0){
        perror("Error writing to serial port");
        return -1;
    }
    printf("%d bytes written\n", res);

    timeoutFlag = 0, timeoutCount = 0, timerFlag = 1;

    while (timeoutCount < connectionParameters.timeOut){
        if(timerFlag){
            alarm(3);
            timerFlag = 0;
        }

        int readResult = getCommand(tx_fd, cmdUA, 5);

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
            int res = write(tx_fd, cmdSet, 5);
            if(res < 0){
                perror("Error writing to serial port");
                return -1;
            }
            printf("%d bytes written\n", res);
            timeoutCount++;
        }
    }    

    return -1;
}

int transmitter_llclose(linkLayer connectionParameters){

    // DISC frame header
    unsigned char cmdDisc[] = {FLAG, A_tx, C_DISC, A_tx ^ C_DISC, FLAG};
    // UA frame header
    unsigned char cmdUA[] = {FLAG, A_rx, C_UA, A_rx ^ C_UA, FLAG};

    (void) signal(SIGALRM, timeOut);

    int res = write(tx_fd, cmdDisc, 5);
    if(res < 0){
        perror("Error writing to serial port");
        return -1;
    }
    printf("Disconnect command sent\n");

    int timeoutCount = 0;

    timeoutFlag = 0, timeoutCount = 0, timerFlag = 1;

    while (timeoutCount < connectionParameters.timeOut){
        if(timerFlag){
            alarm(3);
            timerFlag = 0;
        }

        int readResult = getCommand(tx_fd, cmdDisc, 5);

        if(readResult < 0){
            perror("Error reading command from serial port");
            return -1;
        }
        else if(readResult > 0){ //Success
            signal(SIGALRM, SIG_IGN); //disable interrupt handler
            printf("Received Disconnection confirm, sending UA\n");
            break;
        }

        if(timeoutFlag){
            int res = write(tx_fd, cmdDisc, 5);
            if(res < 0){
                perror("Error writing to serial port");
                return -1;
            }
            printf("Disconnect command sent again\n");
            timeoutCount++;
        }
    }    
    
    if(write(tx_fd, cmdUA, 5) < 0){
        perror("Error writing to serial port");
        return -1;
    }
    printf("UA control sent\n");
    return 1;
}

void timeOut(){
    printf("Connection timeout\n");
    timeoutFlag = 1;    //indicate there was a timeout
    timerFlag = 1;      //restart the timer 
}

#endif