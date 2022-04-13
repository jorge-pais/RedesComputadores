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

    // wrong parameter
    return -1;
    
}

int llclose(int showStatistics){
    /* if(connectionParameters.role == TRANSMITTER){
        return transmitter_llclose(connectionParameters);
    }
    else if(connectionParameters.role == RECEIVER){
        return receiver_llclose(connectionParameters);
    } */

    //somekind of error
    return -1;
}