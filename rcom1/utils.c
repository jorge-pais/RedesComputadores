#include "utils.h"

static struct termios oldtio, newtio;

int configureSerialterminal(linkLayer connectionParameters){

    int fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY);
    
    if (fd < 0){
        printf("Couldn't open %s\n", connectionParameters.serialPort);
        exit(-1);
    }

    //copy current serial port configuration
    if(tcgetattr(fd, &oldtio) == -1){
        printf("couldn't save current port settings\n");
        exit(-1);
    }
    
    //Configure serial port connection
    bzero(&newtio, sizeof(newtio));
    //newtio.c_cflag = connectionParameters.baudRate | CS8 | CLOCAL | CREAD;
    newtio.c_cflag = CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0; 

    //Set local configuration
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 30; // Set the read() timeout for 3 seconds
    newtio.c_cc[VMIN]     = 0;  // Set minimum of characters to be read

    tcflush(fd, TCIOFLUSH); // flush whatever's in the buffer

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    return fd;
}

int getCommand(int fd, unsigned char *cmd, int cmdLen){
    int state = 0, res;
    unsigned char rx_byte;

    while(state != 5){
        res = read(fd, &rx_byte, 1);
        if(res > 0) //Something was read
            printf("received byte: 0x%02x -- state: %d \n", rx_byte, state);
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
            if(rx_byte == (cmd[1]^cmd[2])) //BCC1
                state = 4;
            else if(rx_byte == cmd[0])
                state = 1;
            else
                state = 0;
            break;
            case 4:
            if(rx_byte == cmd[0]) //FLAG
                state = 5;
            else
                state = 0;
            break;
        }
    }
    if(state == 5) //everything OK, we happy
        return 1;
    else if(res < 0) //somekind of error
        return -1;
    
    //didn't receive what was expected
    return 0;
}

int *byteStuffing(unsigned char *data, int dataSize, int *outputDataSize){
    if(data == NULL || outputDataSize == NULL){
        printf("one or more parameters are invalid\n");
        return NULL;
    }
    
}

int convertBaudRate(int baud){
    return BAUDRATE_DEFAULT;
}