#include "linklayer.h"
#include "transmitter.h"
#include "receiver.h"
#include "utils.h"

static int serialRole;

int llopen(linkLayer connectionParameters){
    
    #ifdef testing
    {
    srand(time(NULL));
    }
    #endif
    
    if(connectionParameters.role == TRANSMITTER){
        serialRole = TRANSMITTER;
        return transmitter_llopen(connectionParameters);
    }
    else if(connectionParameters.role == RECEIVER){
        serialRole = RECEIVER;
        return receiver_llopen(connectionParameters);
    }

    // wrong parameter
    return -1;
    
}

int llclose(int showStatistics){
    if(serialRole == TRANSMITTER){
        return transmitter_llclose(showStatistics);
    }
    else if(serialRole == RECEIVER){
        return receiver_llclose(showStatistics);
    }

    //somekind of error
    return -1;
}