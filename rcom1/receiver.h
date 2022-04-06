#ifndef RECEIVER_H
#define RECEIVER_H

#include "utils.h"

/* 
Open logical connection on the receiver end
*/
int receiver_llopen(linkLayer connectionParameters);
int receiver_llclose(linkLayer connectionParameters);

#endif