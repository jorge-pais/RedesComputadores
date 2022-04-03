#include "linklayer.h"
#include "definitions.h"

// Globally declared termios structures
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
    newtio.c_cflag = connectionParameters.baudRate | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0; 

    //Set local configuration
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 30; // Set read() timeout
    newtio.c_cc[VMIN]     = 0;  // Set minimum of characters to be read

    tcflush(fd, TCIOFLUSH); // flush whatever's in the buffer

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    return fd;
}

int receiver_llopen(linkLayer connectionParameters){

    int fd = configureSerialterminal(connectionParameters);

    int state = 0, res = 0;
    unsigned char rx_byte = 0;
    
    while(TRUE){
        while(state != 5){
            res = read(fd, &rx_byte, 1);
            if(res)
                printf("received byte: 0x%02x -- state: %d \n", rx_byte, state);
            else
                break;
            switch(state){ //State machine
                case 0:
                if(rx_byte==FLAG)
                    state = 1;
                else
                    state = 0;
                break;
                case 1:
                if(rx_byte==A_tx)
                    state = 2;
                else if(rx_byte==FLAG)
                    state = 1;
                else 
                    state = 0;
                break;
                case 2:
                if(rx_byte==C_SET)
                    state = 3;
                else if(rx_byte == FLAG)
                    state = 1;
                else
                    state = 0;
                break;
                case 3:
                if(rx_byte == BCC)
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
        if(res == 0) break; //If nothing has been read for longer than 3 seconds

        if(state == 5){
            printf("Received SET, sending UA...\n");
            unsigned char buffer[5] = {FLAG, A_tx, C_UA, BCC, FLAG}; 
            res = write(fd,buffer,5);   
            printf("%d bytes written\n", res);
            break;
        }
    }
}

int receiver_llclose();