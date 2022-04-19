#include "receiver.h"

/* 
Globally declared serial terminal file descriptor
and linklayer parameters
*/
static int rx_fd;
linkLayer *rx_connectionParameters;

//File and time_t for event logs
FILE *rx_stats;
char rx_event_fileName[] = "rx_statistics";
time_t rx_now;

//ERROR COUNTERS for statistics
int stat_rxRejCount = 0;
int stat_rxIFrames = 0;
int stat_duplicatesReceived = 0;

//Last successfully read I frame sequence number
static u_int8_t rx_prevSeqNum = 1;

int receiver_llopen(linkLayer connectionParameters){

    // Save connection parameters
    rx_connectionParameters = checkParameters(connectionParameters); 

    rx_fd = configureSerialterminal(*rx_connectionParameters);

    // open event log file
    rx_stats = fopen(rx_event_fileName, "w");
    if(rx_stats == NULL){
        writeEventToFile(rx_stats, &rx_now, "Error during receiver_llopen() function call\n");
        return -1;
    }

    writeEventToFile(rx_stats, &rx_now, "llopen() called\n");
    
    // We're expecting a SET command from tx
    u_int8_t cmdSET[] = {FLAG, A_tx, C_SET, (A_tx ^ C_SET), FLAG};

    if(checkHeader(rx_fd, cmdSET, 5) <= 0){
        writeEventToFile(rx_stats, &rx_now, "Haven't received SET\n");
        //fprintf(stderr, "Haven't received SET\n");
        return -1;
    }
    writeEventToFile(rx_stats, &rx_now, "Received SET, sending UA\n");
    //printf("Received SET, sending UA\n");

    // UA frame reply
    u_int8_t repUA[] = {FLAG, A_tx, C_UA, (A_tx ^ C_UA), FLAG};

    if(write(rx_fd, repUA, 5) < 0){
        writeEventToFile(rx_stats, &rx_now, "Error writing to serial port\n");
        //fprintf(stderr, "Error writing to serial port");
        return -1;
    }

    writeEventToFile(rx_stats, &rx_now, "Received UA, connection secured\n");
    return 1;
}

int llread(char *packet){
    fputc((int)'\n', rx_stats);
    writeEventToFile(rx_stats, &rx_now, "llread() called\n");
    //DEBUG_PRINT("[llopen() call] %d \n", rx_prevSeqNum);
    if(packet == NULL){
        writeEventToFile(rx_stats, &rx_now, "Error during llread() - invalid parameter\n");
        return -1;
    }

    u_int8_t *datafield = malloc(2*MAX_PAYLOAD_SIZE + 3);
    u_int8_t *destuffedData;

    if(datafield == NULL){
        writeEventToFile(rx_stats, &rx_now, "Error during llread() - datafield memory allocation failed\n");
        return -1;
    }

    //possible reply frames
    u_int8_t repRR[] = {FLAG, A_tx, C_RR(rx_prevSeqNum), (A_tx ^ C_RR(rx_prevSeqNum)), FLAG};
    u_int8_t repREJ[] = {FLAG, A_tx, C_REJ(!rx_prevSeqNum), (A_tx ^ C_REJ(!rx_prevSeqNum)), FLAG};

    
    u_int8_t STOP = 0, rx_byte, BCC2, currSeqNum;
    int res, i, destuffedDataSize;

    while(!STOP){
        res = readControlField(rx_fd, 4);
        if(res == 0xFF || res == C_DISC){
            writeEventToFile(rx_stats, &rx_now, "Error during llread() - Packet's control field invalid\n");
            return -1;
        }

        else if(res != C(0) && res != C(1)){ // not an I frame
            writeEventToFile(rx_stats, &rx_now, "Didn't receive I frame\n");
            return 0;
        }
        
        currSeqNum = I_SEQ(res);

        if (currSeqNum == !rx_prevSeqNum){ //This is a new I frame
            writeEventToFile(rx_stats, &rx_now, "New I frame received\n");
            stat_rxIFrames++;

            // Read stuffed data field, excluding FLAG
            i = 0;
            res = read(rx_fd, &rx_byte, 1);
            if(res < 0){
                writeEventToFile(rx_stats, &rx_now, "Error during llread() - could not read from serial port\n");
                free(datafield);
                return -1;
            }
            do{
                datafield[i++] = rx_byte;
                res = read(rx_fd, &rx_byte, 1);
                if(res < 0){
                    writeEventToFile(rx_stats, &rx_now, "Error during llread() - could not read from serial port\n");
                    free(datafield);
                    return -1;
                }
            } while (rx_byte != FLAG);
            
            destuffedData = byteDestuffing(datafield, i, &destuffedDataSize);

            BCC2 = generateBCC(destuffedData, destuffedDataSize - 1);

            #ifdef test_data_corruption
            {
            if(rand() % test_data_corruption == 0)
                BCC2++;
            }
            #endif

            if(destuffedData[destuffedDataSize-1] == BCC2){
                writeEventToFile(rx_stats, &rx_now, "BCC2 check passed, sending RR\n");
                //free(datafield);
                
                //send RR
                res = write(rx_fd, repRR, 5);
                if(res < 0){
                    writeEventToFile(rx_stats, &rx_now, "Error during llread() - could not write to serial port\n");
                    free(datafield);
                    free(destuffedData);
                    return -1;
                }
                rx_prevSeqNum = !rx_prevSeqNum;
                STOP = 1; //break;
            }
            else{
                writeEventToFile(rx_stats, &rx_now, "BCC2 check failed, sending REJ\n");
                
                free(destuffedData);
                res = write(rx_fd, repREJ, 5);

                stat_rxRejCount++;

                if(res < 0){
                    writeEventToFile(rx_stats, &rx_now, "Error during llread() - could not write to serial port\n");
                    free(datafield);
                    return -1;
                }
                //continue;
            }
        }
        else{ //In case of duplicate I frame
            writeEventToFile(rx_stats, &rx_now, "Duplicate frame received, sending RR\n");
            stat_duplicatesReceived++;

            u_int8_t repRR_rej[] = {FLAG, A_tx, C_RR(!rx_prevSeqNum), (A_tx ^ C_RR(!rx_prevSeqNum)), FLAG};
            res = write(rx_fd, repRR_rej, 5);
            if(res < 0){
                writeEventToFile(rx_stats, &rx_now, "Error during llread() - could not write to serial port\n");
                free(datafield);
                return -1;
            }
                
            do{ //Dummy read, useless
                res = read(rx_fd, &rx_byte, 1);
            } while (rx_byte != FLAG || res == 0);
        }
    }

    writeEventToFile(rx_stats, &rx_now, "Writing read data to packet\n");
    for (int i = 0; i < destuffedDataSize; i++)
        packet[i] = destuffedData[i];

    free(datafield);
    free(destuffedData);

    return (destuffedDataSize - 1);
}

u_int8_t *byteDestuffing(u_int8_t *data, int dataSize, int *outputDataSize){
    if(data == NULL || outputDataSize == NULL){
        writeEventToFile(rx_stats, &rx_now, "Error during byteDestuffing() - invalid parameters in function call\n");
        //fprintf(stderr, "invalid parameters in function call");
        return NULL;
    }
    
    u_int8_t *destuffedData = malloc(dataSize);
    if(destuffedData == NULL){
        writeEventToFile(rx_stats, &rx_now, "Error during byteDestuffing() - destuffedData memory allocation failed\n");
        return NULL;
    }

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
                writeEventToFile(rx_stats, &rx_now, "Error during byteDestuffing() - invalid invalid escape character used\n");
                return NULL;
                break;
            }
    }
    
    if(size != dataSize){
        destuffedData = realloc(destuffedData, size);
        if(destuffedData == NULL){
            writeEventToFile(rx_stats, &rx_now, "Error during byteDestuffing() - memory reallocation failed\n");
            return NULL;
        }
    }

    *outputDataSize = size;
    return destuffedData;
}

int receiver_llclose(int showStatistics){
    fputc((int)'\n', rx_stats);
    writeEventToFile(rx_stats, &rx_now, "llclose() called\n");

    // DISC frame header
    u_int8_t cmdDisc[] = {FLAG, A_tx, C_DISC, A_tx ^ C_DISC, FLAG};
    // UA frame header expected to receive
    u_int8_t cmdUA[] = {FLAG, A_rx, C_UA, A_rx ^ C_UA, FLAG};

    if(checkHeader(rx_fd, cmdDisc, 5) < 0){
        writeEventToFile(rx_stats, &rx_now, "Error reading from serial port\n");
        //fprintf(stderr, "Error reading from serial port");
        return -1;
    }

    writeEventToFile(rx_stats, &rx_now, "Received DISC, sending DISC back\n");
    DEBUG_PRINT("Received DISC, sending DISC back\n");
    
    int res = write(rx_fd, cmdDisc, 5);
    if(res < 0){
        writeEventToFile(rx_stats, &rx_now, "Error writing to serial port\n");
        //fprintf(stderr, "Error writing to serial port");
        return -1;
    }
    DEBUG_PRINT("DISC sent back\n");

    if(checkHeader(rx_fd, cmdUA, 5) < 0){
        writeEventToFile(rx_stats, &rx_now, "Error reading from serial port\n");
        //fprintf(stderr, "Error reading from serial port");
        return -1;
    }

    free(rx_connectionParameters);

    closeSerialterminal(rx_fd);

    fclose(rx_stats);

    if(showStatistics){
        printf("\n######## LINK LAYER STATISTICS ########\n");
        printf("# of I frames received: %d \n", stat_rxIFrames);
        printf("# of REJ frames sent: %d \n", stat_rxRejCount);
        printf("# of duplicate frames received: %d \n", stat_duplicatesReceived);
        
        printf("Open event log using less? [y/n]\n");
        res = getchar();

        if(res=='y'){ //Open file using less
            char command[100] = "less ";
            strcat(command, rx_event_fileName);

            system(command);
        }
    }

    return 1;
}
