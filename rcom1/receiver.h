#ifndef RECEIVER_H
#define RECEIVER_H

#include "utils.h"

/* 
Open logical connection on the receiver end
*/
int receiver_llopen(linkLayer connectionParameters);
//int receiver_llclose(linkLayer connectionParameters);

/*
Perform a byte destuffing operation on vector data

Return values:
    pointer to a new vector, in case of success
    NULL, if somekind of error
*/
u_int8_t *byteDestuffing(u_int8_t *data, int dataSize, int *outputDataSize);

#endif