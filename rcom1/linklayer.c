#include "linklayer.h"
#include "transmitter.c"
#include "receiver.c"
#include "utils.h"

int llopen(linkLayer connectionParameters){

    if(connectionParameters.role == TRANSMITTER){
        return transmitter_llopen(connectionParameters);
    }
    else if(connectionParameters.role == RECEIVER){
        return receiver_llopen(connectionParameters);
    }

    //somekind of error
    return -1;
    
}