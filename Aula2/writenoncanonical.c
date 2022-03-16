/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

#define BAUDRATE B9600
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define A 0x03
#define C 0x03
#define BCC A^C
#define FLAG 0x7E

volatile int STOP=FALSE;

void timeout();
static int timeoutFlag, state; // timeoutFlag = 0, no timeout received byte successfully
                        // timeoutFlag = 1, waiting for SIGALARM while receiving byte stream
                        // timeoutFlag = 2, connection timed out
static int timerFlag;

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255];
    int i, sum = 0, speed = 0;
    
    if ( (argc < 2) || ((strcmp("/dev/ttyS10", argv[1])!=0) && 
        (strcmp("/dev/ttyS11", argv[1])!=0) )) {
        
        printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
        exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */


    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
        perror("tcgetattr");
        exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */



  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    //
    // AULA
    //

    printf("New termios structure set\n");

    printf("Sending SET...\n");
    unsigned char buffer[5] = {FLAG, A, C, BCC, FLAG}; 
    res = write(fd,buffer,5);   
    printf("%d bytes written\n", res);

    unsigned char byte;

    signal(SIGALRM, timeout); //attaches timeout function to interrupt

    timeoutFlag = 1;
    timerFlag = 1;
    state = 0;
    
    while(state != 5){
        /* if(timerFlag){
            alarm(3);
            timerFlag = 0;
        } */
        read(fd, &byte, 1);
        printf("received byte: 0x%02x -- state: %d \n", byte, state);
        switch(state){  //maquina de estados da receção
            case 0:
            if(byte==FLAG)
                state = 1;
            else
                state = 0;
            break;
                    case 1:
            if(byte==A)
                state = 2;
            else if(byte==FLAG)
                state = 1;
            else 
                state = 0;
            break;
            case 2:
            if(byte==C)
                state = 3;
            else if(byte == FLAG)
                state = 1;
            else
                state = 0;
            break;
            case 3:
            if(byte==BCC)
                state = 4;
            else if(byte == FLAG)
                state = 1;
            else
                state = 0;
            break;
            case 4:
            if(byte == FLAG){
                state = 5;
                timeoutFlag = 0;
                signal(SIGALRM, SIG_IGN); // the signal is now ignored 
            }
            else
                state = 0;
            break;
        }
        
        /* if(timeoutFlag == 1){
            res = write(fd,buffer,5);   
            printf("%d bytes written\n", res);
            timeoutFlag = 0;
        } */
    }
    
    printf("Received UA, closing connection\n");

    /*
    for (i = 0; i < 255; i++) {
      buf[i] = 'a';
    }*/

    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);
    return 0;
}

void timeout(){    
    printf("Connection timeout sending SET again\n");
    timeoutFlag = 1; //mark that there was in fact a timeout
    timerFlag = 1; //restarts the timer
}
