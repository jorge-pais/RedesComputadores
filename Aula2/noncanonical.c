/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define BAUDRATE B9600
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define A 0x03
#define C 0x03
#define FLAG 0x7E
#define BCC A^C

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    unsigned char buf[5];

    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS10", argv[1])!=0) && 
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
    // Inicio da Aula
    //
    
    printf("New termios structure set\n");
    
    int state=0;
    int i = 0;
    unsigned char rx_byte = 0;

    printf("Sleeping 10 sec before reading anything");
    sleep(10);

    while(state != 5){
    	read(fd, &rx_byte, 1);
        printf("received byte: 0x%02x -- state: %d \n", rx_byte, state);
        switch(state){ //maquina de estados da receção
            case 0:
            if(rx_byte==FLAG)
                state = 1;
            else
                state = 0;
            break;
            case 1:
            if(rx_byte==A)
                state = 2;
            else if(rx_byte==FLAG)
                state = 1;
            else 
                state = 0;
            break;
            case 2:
            if(rx_byte==C)
                state = 3;
            else if(rx_byte == FLAG)
                state = 1;
            else
                state = 0;
            break;
            case 3:
            if(rx_byte==BCC)
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

    //printf("Received SET, sending UA...\n");    
    printf("Received SET, waiting 8 sec...\n");
    sleep(8);
    printf("Sending UA... \n");

    unsigned char buffer[5] = {FLAG, A, C, BCC, FLAG}; 
    res = write(fd,buffer,5);   
    printf("%d bytes written\n", res);

    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
