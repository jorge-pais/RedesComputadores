#ifndef TRANSMITTER_H
#define TRANSMITTER_H

#include "utils.h"

/* 
Transmitter end of llopen() which is then passed on to the
actual function

Return values
    1 - on successful connection establishment
    -1 - on error
*/
int transmitter_llopen(linkLayer connectionParameters);

/*
Transmitter end of llclose()

Return values
    1 - on success
    -1 - on error
*/
int transmitter_llclose(int showStatistics);

/*
Auxiliary timeout function for handling SIGALRM signals
*/
void timeOut();

/*
Perform a byte stuffing operation on vector data

Return values:
    pointer to a new vector 
    NULL - somekind of error
*/
u_int8_t *byteStuffing(u_int8_t *data, int dataSize, int *outputDataSize);

/*
Prepare an Information Frame 

Return Values:
    pointer to frame byte array - success
    NULL - somekind of error
*/
u_int8_t *prepareInfoFrame(u_int8_t *buf, int bufSize, int *outputSize, u_int8_t sequenceBit);

#endif