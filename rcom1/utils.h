#ifndef UTILS_H
#define UTILS_H

#include "linklayer.h"
#include "definitions.h"

/*  
Globally declared termios structures, and serial terminal 
file descriptor
*/
//static struct termios oldtio, newtio;

/*
Configure serial port terminal I/O
Return values:
    file descriptor id - If successful
*/
int configureSerialterminal(linkLayer connectionParameters);
/*
Close Serial Port Terminal connection upon llclose()
Return values:
     1 - connection closed successfully
    -1 - error
*/
int closeSerialterminal(int fd);
/*
Try to read the header for a given frame, 
Return values:
    1   - the command was read successfully
    0   - couldn´t read anything
    -1  - error while reading
*/
int getCommand(int fd, unsigned char *cmd, int cmdLen);
/*
State machine similar to getCommand() but exclusively used to 
read Information Frame headers

Return values:
    1   - the command was read successfully
    0   - couldn't read anything
    -1  - error while reading
*/
int getInfoCommand(int fd, unsigned char *cmd, int cmdLen);

/*
Convert an integer baudrate to something that termios.h
understands, also checks if a given baud rate is defined
for the current system. This check is done during compiling, so
one should already compile with the target architecture in mind
    eg. 9600 -> B9600 (= 00000015)
If baud is an invalid value, the function will return BAUDRATE_DEFAULT
*/
speed_t convertBaudRate(int baud);

/*
Try to read a especific header for a given frame
Works for Supervision and Unnumbered 
Return values:
    1   - the command was read successfully
    0   - couldn´t read anything
    -1  - error while reading

Mudar nome da função maybe,
Commands são o I, SET, DISC; Replies são o UA, RR, REJ
*/
int getCommand(int fd, unsigned char *cmd, int cmdLen);

/*
Calculate a Block Check Character for a given data vector

Return Values
    BCC - BCC was correctly generated
    -1  - somekind of error
*/
int generateBCC(u_int8_t *data, int dataSize);

#endif