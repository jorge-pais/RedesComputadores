#include "utils.h"

/*  
Globally declared termios structures, and serial terminal 
file descriptor
*/
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
    speed_t baud = convertBaudRate(connectionParameters.baudRate);

    bzero(&newtio, sizeof(newtio));
    //newtio.c_cflag = connectionParameters.baudRate | CS8 | CLOCAL | CREAD;
    newtio.c_cflag = baud | CS8 | CLOCAL | CREAD;
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

int closeSerialterminal(int fd){
    
    tcflush(fd, TCIOFLUSH); // flush whatever's in the buffer

    if (tcsetattr(fd,TCSANOW,&oldtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);
    return 1;
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

int getInfoCommand(int fd, unsigned char *cmd, int cmdLen){
    int state = 0, res;
    unsigned char rx_byte;

    while(state != 4){
        res = read(fd, &rx_byte, 1);
        if(res > 0) //Something was read
            printf("received byte: 0x%02x -- state: %d \n", rx_byte, state);
        else //Nothing has been read or some kind of error
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
            if(rx_byte == (cmd[1]^cmd[2])) //BCC1
                state = 4;
            else if(rx_byte == cmd[0])
                state = 1;
            else
                state = 0;
            break;
        }
    }
    if(state == 4) //everything OK, we happy
        return 1;
    else if(res < 0) //somekind of error
        return -1;
    
    //didn't receive what was expected
    return 0;
}

/*
Later on addicional constant support for SPARC and non-SPARC 
architectures should be implemented
*/
speed_t convertBaudRate(int baud){
    switch (baud)
    {
    case 0:
        #ifndef B0
            return -1;
        #endif
            return B0;
        break;
    case 50:
        #ifndef B50
            return -1;
        #endif
            return B50;
        break;
    case 75:
        #ifndef B75
            return -1;
        #endif
            return B75;
        break;
    case 110:
        #ifndef B110
            return -1;
        #endif
            return B110;
        break;
    case 134:
        #ifndef B134
            return -1;
        #endif
            return B134;
        break;
    case 150:
        #ifndef B150
            return -1;
        #endif
            return B150;
        break;
    case 200:
        #ifndef B200
            return -1;
        #endif
            return B200;
        break;
    case 300:
        #ifndef B300
            return -1;
        #endif
            return B300;
        break;
    case 600:
        #ifndef B600
            return -1;
        #endif
            return B600;
        break;
    case 1200:
        #ifndef B1200
            return -1;
        #endif
            return B1200;
        break;
    case 1800:
        #ifndef B1800
            return -1;
        #endif
            return B1800;
        break;
    case 2400:
        #ifndef B2400
            return -1;
        #endif
            return B2400;
        break;
    case 4800:
        #ifndef B4800
            return -1;
        #endif
            return B4800;
        break;
    case 9600:
        #ifndef B9600
            return -1;
        #endif
            return B9600;
        break;
    case 19200:
        #ifndef B19200
            return -1;
        #endif
            return B19200;
        break;
    case 38400:
        #ifndef B38400
            return -1;
        #endif
            return B38400;
        break;
    case 57600:
        #ifndef B57600
            return -1;
        #endif
            return B57600;
        break;
    case 115200:
        #ifndef B115200
            return -1;
        #endif
            return B115200;
        break;
    case 230400:
        #ifndef B230400
            return -1;
        #endif
            return B230400;
        break;
    case 460800:
        #ifndef B460800
            return -1;
        #endif
            return B460800;
        break;
    case 500000:
        #ifndef B500000
            return -1;
        #endif
            return B500000;
        break;
    case 576000:
        #ifndef B576000
            return -1;
        #endif
            return B576000;
        break;
    case 921600:
        #ifndef B921600
            return -1;
        #endif
            return B921600;
        break;
    case 1000000:
        #ifndef B1000000
            return -1;
        #endif
            return B1000000;
        break;
    case 1152000:
        #ifndef B1152000
            return -1;
        #endif
            return B1152000;
        break;
    case 1500000:
        #ifndef B1500000
            return -1;
        #endif
            return B1500000;
        break;
    case 2000000:
        #ifndef B2000000
            return -1;
        #endif
            return B2000000;
        break;
    default:
        return BAUDRATE_DEFAULT;
        break;
    }

    return BAUDRATE_DEFAULT;
}
