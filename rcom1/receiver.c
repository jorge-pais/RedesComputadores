#include "receiver.h"

/* 
Globally declared serial terminal file descriptor
*/
static int rx_fd;
static int rx_lastSeqNumber = 1;
//u_int8_t timeoutFlag, timerFlag;

int receiver_llopen(linkLayer connectionParameters){

    rx_fd = configureSerialterminal(connectionParameters);

    u_int8_t cmdSET[] = {FLAG, A_tx, C_SET, (A_tx ^ C_SET), FLAG};

    if(checkHeader(rx_fd, cmdSET, 5) <= 0){
        
        perror("Haven't received SET");
        return -1;
    }
    printf("Received SET, sending UA\n");

    u_int8_t cmdUA[] = {FLAG, A_tx, C_UA, (A_tx ^ C_UA), FLAG};

    if(write(rx_fd, cmdUA, 5) < 0){
        perror("Error writing to serial port");
        return -1;
    }

    return 0;
}

int llread(char *packet){
    /* if(packet == NULL)
        return -1; */

    u_int8_t dataFrameHeader[] = {FLAG, A_tx, C(!rx_lastSeqNumber), (A_tx ^ C(!rx_lastSeqNumber))}; //expected header

    //possible response headers
    u_int8_t cmdRR[] = {FLAG, A_tx, C_RR(rx_lastSeqNumber), (A_tx ^ C_RR(rx_lastSeqNumber)), FLAG};
    u_int8_t cmdREJ[] = {FLAG, A_tx, C_REJ(rx_lastSeqNumber), (A_tx ^ C_REJ(rx_lastSeqNumber)), FLAG};
    u_int8_t rx_byte;
    int res;

    u_int8_t STOP = 0;
    while(TRUE){
        res = read(rx_fd, &rx_byte, 1);
        if(res)
            DEBUG_PRINT("received byte: 0x%02x \n", rx_byte);
    }

    return 0;
}

u_int8_t *byteDestuffing(u_int8_t *data, int dataSize, int *outputDataSize){
    if(data == NULL || outputDataSize == NULL){
        printf("invalid parameters in function call");
        return NULL;
    }
    
    u_int8_t *destuffedData = malloc(dataSize);
    if(destuffedData == NULL)
        return NULL;

    int size = 0;
    for (int i = 0; i < dataSize; i++)
    {
        if(data[i] != ESC)
            destuffedData[size++] = data[i];
        else
            switch (data[++i])
            {
            case FLAG^0x20:
                destuffedData[size++] = FLAG;
                break;
            case ESC^0x20:
                destuffedData[size++] = ESC;
                break;
            default: //invalid escape character use
                free(destuffedData);
                return NULL;
                break;
            }
    }
    
    if(size != dataSize){
        destuffedData = realloc(destuffedData, size);
        if(destuffedData == NULL)
            return NULL;
    }

    *outputDataSize = size;
    return destuffedData;
}

/* int receiver_llclose(linkLayer connectionParameters){
    // DISC frame header
    u_int8_t cmdDisc[] = {FLAG, A_tx, C_DISC, A_tx ^ C_DISC, FLAG};
    // UA frame header expected to receive
    u_int8_t cmdUA[] = {FLAG, A_rx, C_UA, A_rx ^ C_UA, FLAG};

    int timeoutCount = 0;

    if(checkHeader(rx_fd, cmdDisc, 3) < 0){
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

        int readResult = checkHeader(rx_fd, cmdUA, 5);

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