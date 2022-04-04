#include "utils.h"

/* 
Globally declared termios structures, and serial terminal 
file descriptor
*/
static struct termios oldtio, newtio;

/*
Configure serial port terminal I/O
*/
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
    newtio.c_cflag = connectionParameters.baudRate | CS8 | CLOCAL | CREAD;
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

/*
Read the header for a given frame, returns 0 if the header has
been read, returns -1 otherwise
*/
int getCommand(int fd, unsigned char *cmd, int cmdLen){
    int state = 0, res;
    unsigned char rx_byte;

    while(state != 5){
        res = read(fd, &rx_byte, 1);
        if(res)
            printf("received byte: 0x%02x -- state: %d \n", rx_byte, state);
        else //Nothing has been read
            break;
        switch(state){ //State machine
            case 0:
            if(rx_byte==cmd[0]) //Flag
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
            if(rx_byte == cmd[1]^cmd[2]) //BCC1
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
    if(state == 5)
        return 0;
    else
        return -1;
}