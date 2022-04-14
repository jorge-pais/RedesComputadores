#include "receiver.h"

/* 
Globally declared serial terminal file descriptor
and linklayer parameters
*/
static int rx_fd;
linkLayer *rx_connectionParameters;

static u_int8_t rx_lastSeqNumber = 0; //Nr = 0, 1
//u_int8_t timeoutFlag, timerFlag;

int receiver_llopen(linkLayer connectionParameters){

    // Save connection parameters
    rx_connectionParameters = checkParameters(connectionParameters); 

    rx_fd = configureSerialterminal(*rx_connectionParameters);

    // We're expecting a SET command from tx
    u_int8_t cmdSET[] = {FLAG, A_tx, C_SET, (A_tx ^ C_SET), FLAG};

    if(checkHeader(rx_fd, cmdSET, 5) <= 0){
        fprintf(stderr, "Haven't received SET");
        return -1;
    }
    printf("Received SET, sending UA\n");

    // Now we've got to send a UA reply to tx
    u_int8_t repUA[] = {FLAG, A_tx, C_UA, (A_tx ^ C_UA), FLAG};

    if(write(rx_fd, repUA, 5) < 0){
        fprintf(stderr, "Error writing to serial port");
        return -1;
    }

    return 0;
}

int llread(char *packet){
    if(packet == NULL)
        return -1;

    u_int8_t dataFrameHeader[] = {FLAG, A_tx, C(rx_lastSeqNumber), (A_tx ^ C(rx_lastSeqNumber))}; //expected header

    //possible response frames
    u_int8_t repRR[] = {FLAG, A_tx, C_RR(!rx_lastSeqNumber), (A_tx ^ C_RR(!rx_lastSeqNumber)), FLAG};
    u_int8_t repREJ[] = {FLAG, A_tx, C_REJ(rx_lastSeqNumber), (A_tx ^ C_REJ(rx_lastSeqNumber)), FLAG};
    
    u_int8_t rx_byte, STOP = 0, duplicateFlag = 0;
    int res, i, destuffedDataSize;

    u_int8_t *dataField = malloc(2*MAX_PAYLOAD_SIZE + 3); 
    /*
    Maybe use double buffering (front and back buffers) for destuffedData
    using globally declared pointers
    We could use this to store the next packet, instead of
    only reading the next frame header and discarding the data field
    */
    u_int8_t *destuffedData;

    while(!STOP){
        res = checkHeader(rx_fd, dataFrameHeader, 4);
        if(res < 0) //In case of error, or 3 seconds have elapsed
            return -1;

        if(!duplicateFlag){ //If we haven't yet received an error-free frame
            rx_byte = 0x00, i = 0;
            while (rx_byte != FLAG){
                res = read(rx_fd, &rx_byte, 1);
                DEBUG_PRINT("[llwrrite() read] 0x%02x\n", rx_byte);
                dataField[i++] = rx_byte;
            }

            //We're only interested in the vector up until BCC2 (length: i-1)
            destuffedData = byteDestuffing(dataField, i - 1, &destuffedDataSize);
            u_int8_t BCC2 = generateBCC(destuffedData, destuffedDataSize - 1);
            DEBUG_PRINT("%2x \n", BCC2);

            if(destuffedData[destuffedDataSize - 1] == BCC2){
                DEBUG_PRINT("BCC2 checks out, sending RR\n");
                free(dataField);
                duplicateFlag = 1;

                res = write(rx_fd, repRR, 5);
                if(res < 0) //maybe change this to the write() size
                    return -1;
            }
            else{
                DEBUG_PRINT("wrong BCC2, sending REJ\n");
                res = write(rx_fd, repREJ, 5);
                if(res < 0)
                    return -1;
            }
        }
        else{ // We've already received an error free duplicate of this frame
            DEBUG_PRINT("Already received this data frame, sending RR\n");
            res = write(rx_fd, repRR, 5);
                if(res < 0) //maybe change this to the write() size
                    return -1;
        }

        //Check if the same data frame is being retransmitted
        DEBUG_PRINT("Checking if we've already received this frame\n");
        res = readSUControlField(rx_fd, 4);
        printf("0x%02x\n", res);

        if(res == 0xFF) //error reading control field
            return -1;
        else if(res == C(!rx_lastSeqNumber) || res == 0xFE) // This is a new frame or nothing could be read
            STOP = 1;
    }
    //Update seq number
    rx_lastSeqNumber = !rx_lastSeqNumber;
    
    //Copy whatever's has been read
    //We assume that char* packet has at least MAX_PAYLOAD_SIZE bytes allocated
    DEBUG_PRINT("writting data to packet\n");
    for (int i = 0; i < destuffedDataSize - 1; i++)
        packet[i] = destuffedData[i];
    free(destuffedData);
    
    return (destuffedDataSize - 1);
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

/* int receiver_llclose(int showStatistics){
    // DISC frame header
    u_int8_t cmdDisc[] = {FLAG, A_tx, C_DISC, A_tx ^ C_DISC, FLAG};
    // UA frame header expected to receive
    u_int8_t cmdUA[] = {FLAG, A_rx, C_UA, A_rx ^ C_UA, FLAG};

    int timeoutCount = 0;

    if(checkHeader(rx_fd, cmdDisc, 3) < 0){
        fprintf(stderr, "Haven't received DISC command");
        return -1;
    }
    printf("Received DISC, sending DISC back\n");

    (void) signal(SIGALRM, timeOut);
    
    int res = write(rx_fd, cmdDisc, 5);
    if(res < 0){
        fprintf(stderr, "Error writing to serial port");
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
            fprintf(stderr, "Error reading command from serial port");
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
                fprintf(stderr, "Error writing to serial port");
                return -1;
            }
            printf("DISC sent back again", res);
            timeoutCount++;
        }
    }    

    return -1;
} */
