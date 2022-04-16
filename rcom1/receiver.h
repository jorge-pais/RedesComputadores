#ifndef RECEIVER_H
#define RECEIVER_H

#include "utils.h"

//static int rx_fd;

/* 
Receiver end of llopen() which is then passed on to the
actual function

Return values
    1 - on successful connection establishment
    -1 - on error
*/
int receiver_llopen(linkLayer connectionParameters);

/*
Transmitter end of llclose()

Return values
    1 - on success
    -1 - on error
*/
int receiver_llclose(int showStatistics);

/*
Perform a byte destuffing operation on vector data

Return values:
    pointer to a new vector, in case of success
    NULL, if somekind of error
*/
u_int8_t *byteDestuffing(u_int8_t *data, int dataSize, int *outputDataSize);

#endif