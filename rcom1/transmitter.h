#ifndef TRANSMITTER_H
#define TRANSMITTER_H

#include "utils.h"

int transmitter_llopen(linkLayer connectionParameters);
int transmitter_llclose(linkLayer connectionParameters);
void timeOut();

/*
Perform a byte stuffing operation on vector data

Return values:
    pointer to a new vector 
    NULL - somekind of error
*/
u_int8_t *byteStuffing(unsigned char *data, int dataSize, int *outputDataSize);

/*
Prepare an Information Frame 
*/
u_int8_t *prepareInfoFrame(char *buf, int bufSize, int *outputSize, u_int8_t sequenceBit);

#endif