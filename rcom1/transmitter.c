#include "transmitter.h"

/* 
Globally declared serial terminal file descriptor
and linklayer connection parameters
*/
static int tx_fd;
linkLayer *tx_connectionParameters;

//File for event logs
FILE *tx_stats;
char tx_event_fileName[] = "tx_statistics";
time_t tx_now;

//ERROR COUNTERS for statistics
int stat_txRejCount = 0;
int stat_txIFrames = 0;
int stat_timeOutsCount = 0; //different from timeoutCount
int stat_retransmittionCount = 0;

// llwrite()
static u_int8_t tx_currSeqNumber = 0; // Ns = 0, 1

//timeout related functions
u_int8_t timeoutFlag, timerFlag, timeoutCount;

int transmitter_llopen(linkLayer connectionParameters){

    tx_connectionParameters = checkParameters(connectionParameters);
    
    tx_fd = configureSerialterminal(*tx_connectionParameters);

    // open event log file
    tx_stats = fopen(tx_event_fileName, "w");
    if(tx_stats == NULL){
        writeEventToFile(tx_stats, &tx_now, "Error opening statistics file\n");
        return -1;
    }

    writeEventToFile(tx_stats, &tx_now, "llopen() called\n");

    // SET frame header
    u_int8_t cmdSet[] = {FLAG, A_tx, C_SET, (A_tx ^ C_SET), FLAG};
    // UA frame header, what we are expecting to receive
    u_int8_t cmdUA[] = {FLAG, A_tx, C_UA, (A_tx ^ C_UA), FLAG};

    (void) signal(SIGALRM, timeOut);

    int res = write(tx_fd, cmdSet, 5);
    if(res < 0){
        writeEventToFile(tx_stats, &tx_now, "Error writing to serial port\n");
        fclose(tx_stats);
        return -1;
    }
    writeEventToFile(tx_stats, &tx_now, "Sending SET command\n");
    //printf("%d bytes written\n", res);

    timeoutFlag = 0, timeoutCount = 0, timerFlag = 1;

    while (timeoutCount < tx_connectionParameters->numTries){
        if(timerFlag){
            alarm(tx_connectionParameters->timeOut);
            timerFlag = 0;
        }

        int readResult = checkHeader(tx_fd, cmdUA, 5);

        if(readResult < 0){
            writeEventToFile(tx_stats, &tx_now, "Error reading command from serial port\n");
            return -1;
        }
        else if(readResult > 0){ //Success
            writeEventToFile(tx_stats, &tx_now, "Connection established\n");
            signal(SIGALRM, SIG_IGN); //disable interrupt handler
            return 1;
        }

        if(timeoutFlag){
            int res = write(tx_fd, cmdSet, 5);
            if(res < 0){
                writeEventToFile(tx_stats, &tx_now, "Error writing to serial port\n");
                fclose(tx_stats);
                return -1;
            }
            writeEventToFile(tx_stats, &tx_now, "Sending SET command again\n");

            timeoutCount++;
            stat_timeOutsCount++; //total number of timeouts
            timeoutFlag = 0;
        }
    }    

    writeEventToFile(tx_stats, &tx_now, "Failed to establish connection\n");
    fclose(tx_stats);

    return -1;
}

u_int8_t *prepareInfoFrame(u_int8_t *buf, int bufSize, int *outputSize, u_int8_t sequenceBit){
    if(buf == NULL || bufSize <= 0 || bufSize > MAX_PAYLOAD_SIZE){
        writeEventToFile(tx_stats, &tx_now, "prepareIntoForm() - invalid parameters\n");
        return NULL;
    }
    // Prepare the frame data
    u_int8_t *data = malloc(bufSize + 1);
    if(data == NULL){
        writeEventToFile(tx_stats, &tx_now, "prepareIntoForm() - data memory allocation failed\n");
        return NULL;
    }
    
    for (int i = 0; i < bufSize; i++) // Copy the buffer
        data[i] = buf[i];
    // Add the BCC byte
    data[bufSize] = (u_int8_t) generateBCC((u_int8_t*) buf, bufSize);

    int stuffedSize = 0;
    u_int8_t *stuffedData = byteStuffing(data, bufSize+1, &stuffedSize);
    if(stuffedData == NULL){
        writeEventToFile(tx_stats, &tx_now, "prepareIntoForm() - byte stuffing failed\n");
        return NULL;
    }
    free(data);

    u_int8_t *outgoingData = malloc(stuffedSize + 5);
    if(outgoingData == NULL){
        writeEventToFile(tx_stats, &tx_now, "prepareIntoForm() - outgoingData memory allocation failed\n");
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
    fputc((int)'\n', tx_stats);
    writeEventToFile(tx_stats, &tx_now, "llwrite() called\n");
    if(buf == NULL || bufSize > MAX_PAYLOAD_SIZE)
        return -1;

    int frameSize = 0;
    u_int8_t *frame = prepareInfoFrame((u_int8_t*) buf, bufSize, &frameSize, tx_currSeqNumber);
    
    //DEBUG_PRINT("[llopen() start] SEQ NUMBER: %d\n", tx_currSeqNumber);
    
    // Write for the first time
    int res = write(tx_fd, frame, frameSize);
    if(res < 0){
        writeEventToFile(tx_stats, &tx_now, "Error writing to serial port\n");
        free(frame);
        return -1;
    }
    stat_txIFrames++;

    writeEventToFile(tx_stats, &tx_now, "Written ");
    fprintf(tx_stats, "%d bytes to serial port\n", res);
    
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

        #ifdef test_missing_su_frame
        {
            if(rand() % test_missing_su_frame == 0)
                control = 0x03; // Simulate dropped packet
        }
        #endif

        //Check if the header and the sequence number are valid
        if(control == C_RR(!tx_currSeqNumber)){ //Receive receipt
            writeEventToFile(tx_stats, &tx_now, "Received RR\n");
            tx_currSeqNumber = !tx_currSeqNumber;

            (void) signal(SIGALRM, SIG_IGN); //disable signal handler
            free(frame);
            return bufSize;
        }
        else if(control == C_REJ(tx_currSeqNumber)){ //REJ
            res = write(tx_fd, frame, frameSize);
            if(res < 0){
                writeEventToFile(tx_stats, &tx_now, "Frame retransmission failed\n");
                free(frame);
                return -1;
            }
            writeEventToFile(tx_stats, &tx_now, "(Retransmission) Written ");
            fprintf(tx_stats, "%d bytes to serial port\n", res);

            stat_txRejCount++;
            stat_txIFrames++;
            stat_retransmittionCount++;

            timeoutCount = 0;

            alarm(tx_connectionParameters->timeOut); // alarm reset
        }

        if(timeoutFlag){
            res = write(tx_fd, frame, frameSize);
            if(res < 0){
                writeEventToFile(tx_stats, &tx_now, "Frame retransmission failed\n");
                free(frame);
                return -1;
            }
            writeEventToFile(tx_stats, &tx_now, "(Retransmission) Written ");
            fprintf(tx_stats, "%d bytes to serial port\n", res);

            stat_txIFrames++;
            stat_retransmittionCount++;
            stat_timeOutsCount++;

            timeoutCount++;
            timeoutFlag = 0;
        }
    }
    (void) signal(SIGALRM, SIG_IGN); //disable signal handler
    
    return -1;
}

int transmitter_llclose(int showStatistics){
    fputc((int)'\n', tx_stats);
    writeEventToFile(tx_stats, &tx_now, "llclose() called\n");

    // DISC frame header
    u_int8_t cmdDisc[] = {FLAG, A_tx, C_DISC, A_tx ^ C_DISC, FLAG};
    // UA frame header
    u_int8_t cmdUA[] = {FLAG, A_rx, C_UA, A_rx ^ C_UA, FLAG};

    (void) signal(SIGALRM, timeOut);

    int res = write(tx_fd, cmdDisc, 5);
    if(res < 0){
        writeEventToFile(tx_stats, &tx_now, "Error writing to serial port\n");
        fclose(tx_stats);
        return -1;
    }
    writeEventToFile(tx_stats, &tx_now, "Sent SET\n");
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
            writeEventToFile(tx_stats, &tx_now, "Error writing to serial port\n");
            fclose(tx_stats);
            return -1;
        }
        else if(res > 0){ //Success
            signal(SIGALRM, SIG_IGN); //disable interrupt handler
            writeEventToFile(tx_stats, &tx_now, "Received DISC, sending UA\n");
            //DEBUG_PRINT("Received Disconnection confirm, sending UA\n");
            break;
        }

        if(timeoutFlag){
            res = write(tx_fd, cmdDisc, 5);
            if(res < 0){
                writeEventToFile(tx_stats, &tx_now, "Error writing to serial port\n");
                fclose(tx_stats);
                return -1;
            }

            writeEventToFile(tx_stats, &tx_now, "DISC command sent again\n");
            //DEBUG_PRINT("Disconnect command sent again\n");
            timeoutCount++;
            stat_timeOutsCount++;
        }
    }

    if(write(tx_fd, cmdUA, 5) < 0){
        writeEventToFile(tx_stats, &tx_now, "Error writing to serial port\n");
        fclose(tx_stats);
        return -1;
    }
    writeEventToFile(tx_stats, &tx_now, "UA control sent\n");

    free(tx_connectionParameters);
    closeSerialterminal(tx_fd);

    fclose(tx_stats);

    if(showStatistics){
        printf("\n######## LINK LAYER STATISTICS ########\n");
        printf("# of I frames sent: %d\n", stat_txIFrames);
        printf("# of total connection timeouts: %d\n", stat_timeOutsCount);
        printf("# of REJ frames received: %d\n", stat_txRejCount);
        printf("# of retransmitted frames: %d\n", stat_retransmittionCount);
        
        printf("Open event log using less? [y/n]\n");
        res = getchar();

        if(res == 'y'){
            char command[100] = "less ";
            strcat(command, tx_event_fileName);

            system(command);
        }
    }

    return 1;
}

u_int8_t *byteStuffing(u_int8_t *data, int dataSize, int *outputDataSize){
    if(data == NULL || outputDataSize == NULL || dataSize < 1){
        writeEventToFile(tx_stats, &tx_now, "byteStuffing() - one or more parameters are invalid\n");
        return NULL;
    }

    // Maximum possible stuffed data size is twice that of the input data array
    // We prevent having to reallocate memory during stuffing
    u_int8_t *stuffedData = malloc(2*dataSize); 
    if(stuffedData == NULL){
        writeEventToFile(tx_stats, &tx_now, "byteStuffing() - stuffedData memory allocation failed\n");
        return NULL;
    }

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
        if(stuffedData == NULL){
            writeEventToFile(tx_stats, &tx_now, "byteStuffing() - stuffedData memory reallocation failed\n");
            return NULL;
        }
    }
    *outputDataSize = size;
    return stuffedData;
}

void timeOut(){
    writeEventToFile(tx_stats, &tx_now, "Connection timeout\n");
    //printf("Connection timeout\n");
    timeoutFlag = 1;    //indicate there was a timeout
    timerFlag = 1;      //restart the timer 
}