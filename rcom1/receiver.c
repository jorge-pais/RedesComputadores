#include "linklayer.h"

int receiver_llopen(linkLayer connectionParameters){

    int fd, c, res;
    struct termios oldtio, newtio;

    fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY);
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

    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 30;
    newtio.c_cc[VMIN]     = 0;

    tcflush(fd, TCIOFLUSH); // flush whatever 

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    

}