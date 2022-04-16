#include "transmitter.h"

/* 
Globally declared serial terminal file descriptor
and linklayer connection parameters
*/
static int tx_fd;
linkLayer *tx_connectionParameters;

static u_int8_t tx_currSeqNumber = 0; // Ns = 0, 1
u_int8_t timeoutFlag, timerFlag, timeoutCount;

int transmitter_llopen(linkLayer connectionParameters){

    tx_connectionParameters = checkParameters(connectionParameters);
    
    tx_fd = configureSerialterminal(*tx_connectionParameters);

    // SET frame header
    u_int8_t cmdSet[] = {FLAG, A_tx, C_SET, (A_tx ^ C_SET), FLAG};
    // UA frame header, what we are expecting to receive
    u_int8_t cmdUA[] = {FLAG, A_tx, C_UA, (A_tx ^ C_UA), FLAG};

    (void) signal(SIGALRM, timeOut);

    int res = write(tx_fd, cmdSet, 5);
    if(res < 0){
        fprintf(stderr, "Error writing to serial port");
        return -1;
    }
    printf("%d bytes written\n", res);

    timeoutFlag = 0, timeoutCount = 0, timerFlag = 1;

    while (timeoutCount < tx_connectionParameters->numTries){
        if(timerFlag){
            alarm(tx_connectionParameters->timeOut);
            timerFlag = 0;
        }

        int readResult = checkHeader(tx_fd, cmdUA, 5);

        if(readResult < 0){
            fprintf(stderr, "Error reading command from serial port");
            return -1;
        }
        else if(readResult > 0){ //Success
            signal(SIGALRM, SIG_IGN); //disable interrupt handler
            //printf("Received UA, connection established\n");
            return 1;
        }

        if(timeoutFlag){
            int res = write(tx_fd, cmdSet, 5);
            if(res < 0){
                fprintf(stderr, "Error writing to serial port");
                return -1;
            }
            printf("%d bytes written\n", res);
            timeoutCount++;
            timeoutFlag = 0;
        }
    }    

    return -1;
}

u_int8_t *prepareInfoFrame(u_int8_t *buf, int bufSize, int *outputSize, u_int8_t sequenceBit){
    if(buf == NULL || bufSize <= 0 || bufSize > MAX_PAYLOAD_SIZE)
        return NULL;

    // Prepare the frame data
    u_int8_t *data = malloc(bufSize + 1);
    if(data == NULL)
        return NULL;
    
    for (int i = 0; i < bufSize; i++) // Copy the buffer
        data[i] = buf[i];
    // Add the BCC byte
    data[bufSize] = (u_int8_t) generateBCC((u_int8_t*) buf, bufSize);

    int stuffedSize = 0;
    u_int8_t *stuffedData = byteStuffing(data, bufSize+1, &stuffedSize);
    if(stuffedData == NULL)
        return NULL;

    free(data);

    u_int8_t *outgoingData = malloc(stuffedSize + 5);
    if(outgoingData == NULL){
        free(stuffedData);
        return NULL;
    }
    outgoingData[0] = FLAG;
    outgoingData[1] = A_tx;
    outgoingData[2] = C(sequenceBit);
    outgoingData[3] = A_tx ^ C(sequenceBit);

    // Copy the stuffed data
    for (int i = 0; i < stuffedSize; i++)
        outgoingData[i+4] = stuffedData[i];
    
    // Add a trailling FLAG
    outgoingData[stuffedSize + 4] = FLAG;

    free(stuffedData);

    *outputSize = stuffedSize + 5;
    return outgoingData;
}

int llwrite(char *buf, int bufSize){
    if(buf == NULL || bufSize > MAX_PAYLOAD_SIZE)
        return -1;

    int frameSize = 0;
    u_int8_t *frame = prepareInfoFrame((u_int8_t*) buf, bufSize, &frameSize, tx_currSeqNumber);
    
    DEBUG_PRINT("[llopen() start] SEQ NUMBER: %d\n", tx_currSeqNumber);
    // Write for the first time
    int res = write(tx_fd, frame, frameSize);
    if(res < 0){
        fprintf(stderr, "error writing to serial port");
        free(frame);
        return -1;
    }
    printf("%d bytes written\n", res);

    u_int8_t control;

    (void) signal(SIGALRM, timeOut); // Set up signal handler

    //Cycle through timeouts
    timeoutFlag = 0; timerFlag = 1; timeoutCount = 0;
    while (timeoutCount < tx_connectionParameters->numTries){
        if(timerFlag){
            alarm(tx_connectionParameters->timeOut);
            timerFlag = 0;
        }
        //read the incoming frame control field
        control = readControlField(tx_fd, 5);

        //Check if the header and the sequence number are valid
        if(control == C_RR(!tx_currSeqNumber)){ //Receive receipt
            printf("transmission successful\n");
            tx_currSeqNumber = !tx_currSeqNumber;

            (void) signal(SIGALRM, SIG_IGN); //disable signal handler
            return bufSize;
        }
        else if(control == C_REJ(tx_currSeqNumber)){ //REJ
            res = write(tx_fd, frame, frameSize);
            if(res < 0){
                free(frame);
                return -1;
            }
            printf("%d bytes written\n", res);

            timeoutCount = 0;

            alarm(tx_connectionParameters->timeOut); // alarm reset
        }

        if(timeoutFlag){
            res = write(tx_fd, frame, frameSize);
            if(res < 0){
                return -1;
            }
            printf("%d bytes written\n", res);
            timeoutCount++;
            timeoutFlag = 0;
        }
    }
    (void) signal(SIGALRM, SIG_IGN); //disable signal handler
    
    return -1;
}

int transmitter_llclose(int showStatistics){

    // DISC frame header
    u_int8_t cmdDisc[] = {FLAG, A_tx, C_DISC, A_tx ^ C_DISC, FLAG};
    // UA frame header
    u_int8_t cmdUA[] = {FLAG, A_rx, C_UA, A_rx ^ C_UA, FLAG};

    (void) signal(SIGALRM, timeOut);

    int res = write(tx_fd, cmdDisc, 5);
    if(res < 0){
        fprintf(stderr, "Error writing to serial port");
        return -1;
    }
    DEBUG_PRINT("Disconnect command sent\n");

    int timeoutCount = 0;

    timeoutFlag = 0, timeoutCount = 0, timerFlag = 1;

    while (timeoutCount < tx_connectionParameters->numTries){
        if(timerFlag){
            alarm(tx_connectionParameters->timeOut);
            timerFlag = 0;
        }

        res = checkHeader(tx_fd, cmdDisc, 5);

        if(res < 0){
            fprintf(stderr, "Error reading from serial port");
            return -1;
        }
        else if(res > 0){ //Success
            signal(SIGALRM, SIG_IGN); //disable interrupt handler
            DEBUG_PRINT("Received Disconnection confirm, sending UA\n");
            break;
        }

        if(timeoutFlag){
            res = write(tx_fd, cmdDisc, 5);
            if(res < 0){
                fprintf(stderr, "Error writing to serial port");
                return -1;
            }
            DEBUG_PRINT("Disconnect command sent again\n");
            timeoutCount++;
        }
    }

    if(write(tx_fd, cmdUA, 5) < 0){
        fprintf(stderr, "Error writing to serial port");
        return -1;
    }
    DEBUG_PRINT("UA control sent\n");

    sleep(1);

    free(tx_connectionParameters);
    closeSerialterminal(tx_fd);

    return 1;
}

u_int8_t *byteStuffing(u_int8_t *data, int dataSize, int *outputDataSize){
    if(data == NULL || outputDataSize == NULL){
        fprintf(stderr, "one or more parameters are invalid\n");
        return NULL;
    }

    // Maximum possible stuffed data size is twice that of the input data array
    // We prevent having to reallocate memory during stuffing
    u_int8_t *stuffedData = malloc(2*dataSize); 
    if(stuffedData == NULL)
        return NULL;
    
    int size = 0;

    for (int i = 0; i < dataSize; i++){
        switch (data[i])
        {
        case FLAG:
            stuffedData[size++] = ESC;
            stuffedData[size++] = FLAG ^ 0x20;
            break;
        case ESC:
            stuffedData[size++] = ESC;
            stuffedData[size++] = ESC ^ 0x20;  
            break;
        default:
            stuffedData[size++] = data[i];
            break;
        }
    }

    // Trim the array in memory if needed
    if(size != 2*dataSize){
        stuffedData = realloc(stuffedData, size);
        if(stuffedData == NULL)
            return NULL;
    }
    *outputDataSize = size;
    return stuffedData;
}

void timeOut(){
    printf("Connection timeout\n");
    timeoutFlag = 1;    //indicate there was a timeout
    timerFlag = 1;      //restart the timer 
}