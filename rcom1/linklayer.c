#include "linklayer.h"
#include "transmitter.h"
#include "receiver.h"
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