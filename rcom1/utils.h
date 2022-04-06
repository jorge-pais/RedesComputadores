#ifndef UTILS_H
#define UTILS_H

#include "linklayer.h"
#include "definitions.h"

/*  
Globally declared termios structures, and serial terminal 
file descriptor
*/
static struct termios oldtio, newtio;

/*
Configure serial port terminal I/O
Return values:
    file descriptor - If successful
*/
int configureSerialterminal(linkLayer connectionParameters);
/*
Convert an integer baudrate to something that termios.h
understands
    eg. 9600 -> B9600 (= 00000015)
although this might be system dependent :/
*/
int convertBaudRate(int baud);
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

#endif