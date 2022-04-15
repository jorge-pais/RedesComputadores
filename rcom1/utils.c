#include "utils.h"

/*  
Globally declared termios structures
*/
struct termios oldtio, newtio;

int configureSerialterminal(linkLayer connectionParameters){

    int fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY);
    
    if (fd < 0){
        fprintf(stderr, "Couldn't open %s\n", connectionParameters.serialPort);
        exit(-1);
    }

    //copy current serial port configuration
    if(tcgetattr(fd, &oldtio) == -1){
        fprintf(stderr, "couldn't save current port settings\n");
        exit(-1);
    }
    
    //Configure serial port connection
    speed_t baud = convertBaudRate(connectionParameters.baudRate);

    bzero(&newtio, sizeof(newtio));
    //newtio.c_cflag = connectionParameters.baudRate | CS8 | CLOCAL | CREAD;
    newtio.c_cflag = baud | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0; 

    //Set local configuration
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = (connectionParameters.timeOut * 10); // Set the read() timeout for 3 seconds
    newtio.c_cc[VMIN]     = 0;  // Set minimum of characters to be read

    tcflush(fd, TCIOFLUSH); // flush whatever's in the buffer

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
        fprintf(stderr, "tcsetattr");
        exit(-1);
    }

    return fd;
}

int closeSerialterminal(int fd){
    
    tcflush(fd, TCIOFLUSH); // flush whatever's in the buffer

    if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
        fprintf(stderr, "tcsetattr");
        exit(-1);
    }

    close(fd);
    return 1;
}

int checkHeader(int fd, u_int8_t *cmd, int cmdLen){
    if(cmd == NULL || cmdLen < 4 || cmdLen > 5)
        return -1;

    int state = 0, res;
    u_int8_t rx_byte;

    while(state < cmdLen){
        res = read(fd, &rx_byte, 1);
        if(res > 0) //Something was read
            DEBUG_PRINT("[checkHeader()] received byte: 0x%02x -- state: %d \n", rx_byte, state);
        else //Nothing has been read or some kind of error
            break;
        switch(state){ //State machine
            case 0:
            if(rx_byte==cmd[0]) //FLAG
                state = 1;
            else
                state = 0;
            break;
            case 1:
            if(rx_byte==cmd[1]) //Address field
                state = 2;
            else if(rx_byte==cmd[0])
                state = 1;
            else 
                state = 0;
            break;
            case 2:
            if(rx_byte==cmd[2]) //Control field
                state = 3;
            else if(rx_byte == cmd[0])
                state = 1;
            else
                state = 0;
            break;
            case 3:
            if(rx_byte == cmd[3]) //BCC1
                state = 4;
            else if(rx_byte == cmd[0])
                state = 1;
            else
                state = 0;
            break;
            case 4: //In case there are 5 elements
            if(rx_byte == cmd[0]) //FLAG
                state = 5;
            else
                state = 0;
            break;
        }
    }

    if(state == cmdLen) //everything OK
        return 1;
    else if(res < 0) //somekind of error
        return -1;
    
    //read() had nothing to read
    return 0;
}

u_int8_t readSUControlField(int fd, int cmdLen){
    
    u_int8_t controlField, rx_byte;
    int state = 0, res;

    while(state != cmdLen){
        res = read(fd, &rx_byte, 1);
        if(res > 0) //Something was read after 3 seconds
            DEBUG_PRINT("[readSUControlField()]received byte: 0x%02x -- state: %d \n", rx_byte, state);
        else //Nothing has been read or some kind of error
            break;
        switch(state){ //State machine
            case 0: //Flag
            if(rx_byte==FLAG) 
                state = 1;
            else
                state = 0;
            break;
            case 1: //Address field
            if(rx_byte==A_tx) 
                state = 2;
            else if(rx_byte==FLAG)
                state = 1;
            else 
                state = 0;
            break;
            case 2: //Control field
            if(rx_byte==FLAG) 
                state = 1;
            else{
                state = 3;
                controlField = rx_byte;
            }
            break;
            case 3:
            if(rx_byte == (controlField^A_tx)) //BCC1
                state = 4;
            else if(rx_byte == FLAG)
                state = 1;
            else
                state = 0;
            break;
            case 4:
            if(rx_byte == FLAG)
                state = 5;
            else
                state = 0;
            break;
        }
    }
    if(state == cmdLen)
        if((controlField & 0x01) == 0x01 || (controlField & 0x05) == 0x05 || (controlField == 0x00 || controlField == 0x02)) //Check 
            return controlField;
    
    if(res == 0)
        return 0xFE;
    // In case nothing could be read, or was wrong
    return 0xFF;
}

u_int8_t generateBCC(u_int8_t *data, int dataSize){
    
    if(dataSize == 1) //in case of only one element in the vector
        return data[0];

    u_int8_t BCC = (data[0] ^ data[1]);

    for (int i = 2; i < dataSize; i++)
        BCC ^= data[i];

    return BCC;
}

linkLayer *checkParameters(linkLayer link){
    
    linkLayer *aux = malloc(sizeof(linkLayer));
    if(aux == NULL)
        return NULL;

    aux->baudRate = link.baudRate;

    if(link.role != TRANSMITTER && link.role != RECEIVER)
        aux->role = NOT_DEFINED;
    else
        aux->role = link.role;

    if(link.numTries < 1)
        aux->numTries = MAX_RETRANSMISSIONS_DEFAULT;
    else
        aux->numTries = link.numTries;

    if(link.timeOut < 0)
        aux->timeOut = TIMEOUT_DEFAULT;
    else
        aux->timeOut = link.timeOut;
    
    strcpy(aux->serialPort, link.serialPort);

    return aux;
}

speed_t convertBaudRate(int baud){
    switch (baud)
    {
    case 0:
        #ifndef B0
            return BAUDRATE_DEFAULT;
        #else
            return B0;
        #endif
        break;
    case 50:
        #ifndef B50
            return BAUDRATE_DEFAULT;
        #else
            return B50;
        #endif
        break;
    case 75:
        #ifndef B75
            return BAUDRATE_DEFAULT;
        #else
            return B75;
        #endif
        break;
    case 110:
        #ifndef B110
            return BAUDRATE_DEFAULT;
        #else
            return B110;
        #endif
        break;
    case 134:
        #ifndef B134
            return BAUDRATE_DEFAULT;
        #else
            return B134;
        #endif
        break;
    case 150:
        #ifndef B150
            return BAUDRATE_DEFAULT;
        #else
            return B150;
        #endif
        break;
    case 200:
        #ifndef B200
            return BAUDRATE_DEFAULT;
        #else
            return B200;
        #endif
        break;
    case 300:
        #ifndef B300
            return BAUDRATE_DEFAULT;
        #else
            return B300;
        #endif
        break;
    case 600:
        #ifndef B600
            return BAUDRATE_DEFAULT;
        #else
            return B600;
        #endif
        break;
    case 1200:
        #ifndef B1200
            return BAUDRATE_DEFAULT;
        #else
            return B1200;
        #endif
        break;
    case 1800:
        #ifndef B1800
            return BAUDRATE_DEFAULT;
        #else
            return B1800;
        #endif
        break;
    case 2400:
        #ifndef B2400
            return BAUDRATE_DEFAULT;
        #else
            return B2400;
        #endif
        break;
    case 4800:
        #ifndef B4800
            return BAUDRATE_DEFAULT;
        #else
            return B4800;
        #endif
        break;
    case 9600:
        #ifndef B9600
            return BAUDRATE_DEFAULT;
        #else
            return B9600;
        #endif
        break;
    case 19200:
        #ifndef B19200
            return BAUDRATE_DEFAULT;
        #else
            return B19200;
        #endif
        break;
    case 38400:
        #ifndef B38400
            return BAUDRATE_DEFAULT;
        #else
            return B38400;
        #endif
        break;
    case 57600:
        #ifndef B57600
            return BAUDRATE_DEFAULT;
        #else
            return B57600;
        #endif
        break;
    case 115200:
        #ifndef B115200
            return BAUDRATE_DEFAULT;
        #else
            return B115200;
        #endif
        break;
    case 230400:
        #ifndef B230400
            return BAUDRATE_DEFAULT;
        #else
            return B230400;
        #endif
        break;
    case 460800:
        #ifndef B460800
            return BAUDRATE_DEFAULT;
        #else
            return B460800;
        #endif
        break;
    case 500000:
        #ifndef B500000
            return BAUDRATE_DEFAULT;
        #else
            return B500000;
        #endif
        break;
    case 576000:
        #ifndef B576000
            return BAUDRATE_DEFAULT;
        #else
            return B576000;
        #endif
        break;
    case 921600:
        #ifndef B921600
            return BAUDRATE_DEFAULT;
        #else
            return B921600;
        #endif
        break;
    case 1000000:
        #ifndef B1000000
            return BAUDRATE_DEFAULT;
        #else
            return B1000000;
        #endif
        break;
    case 1152000:
        #ifndef B1152000
            return BAUDRATE_DEFAULT;
        #else
            return B1152000;
        #endif
        break;
    case 1500000:
        #ifndef B1500000
            return BAUDRATE_DEFAULT;
        #else
            return B1500000;
        #endif
        break;
    case 2000000:
        #ifndef B2000000
            return BAUDRATE_DEFAULT;
        #else
            return B2000000;
        #endif
        break;
    // SPARC architecture
    case 76800:
        #ifndef B76800
            return BAUDRATE_DEFAULT;
        #else
            return B76800;
        #endif
        break;
    case 153600:
        #ifndef B153600
            return BAUDRATE_DEFAULT;
        #else
            return B153600;
        #endif
        break;
    case 307200:
        #ifndef B307200
            return BAUDRATE_DEFAULT;
        #else
            return B307200;
        #endif
        break;
    case 614400:
        #ifndef B614400
            return BAUDRATE_DEFAULT;
        #else
            return B614400;
        #endif
        break;
    //non-SPARC architectures (not in POSIX)
    case 2500000:
        #ifndef B2500000
            return BAUDRATE_DEFAULT;
        #else
            return B2500000;
        #endif
        break;
    case 3000000:
        #ifndef B3000000
            return BAUDRATE_DEFAULT;
        #else
            return B3000000;
        #endif
        break;
    case 3500000:
        #ifndef B3500000
            return BAUDRATE_DEFAULT;
        #else
            return B3500000;
        #endif
        break;
    case 4000000:
        #ifndef B4000000
            return BAUDRATE_DEFAULT;
        #else
            return B4000000;
        #endif
        break;
    default:
        return BAUDRATE_DEFAULT;
        break;
    }

    return BAUDRATE_DEFAULT;
}
