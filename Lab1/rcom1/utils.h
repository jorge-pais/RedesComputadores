#ifndef UTILS_H
#define UTILS_H

#include <time.h>
#include "linklayer.h"
#include "definitions.h"

/*
Configure serial port terminal I/O using linkLayer
Return values:
    file descriptor id - If successful

In case of error the whole program is terminated
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
Tries to read a specific header for a given frame, only works
for valid header lenghts of either 4 or 5 bytes

This function is protected by a timer, after 3 seconds of
if nothing is read it'll return 0

Return values:
    1   - the command was read successfully
    0   - couldn´t read anything
    -1  - error while reading
*/
int checkHeader(int fd, u_int8_t *cmd, int cmdLen);

/*
Tries to read and then output the control field of a
supervision or control frame header

Return values:
    frame control field - the header was read successfully
    0xFE - nothing was read
    0xFF - error while reading
*/
u_int8_t readControlField(int fd, int cmdLen);

/*
Convert an int to the apropriate speed_t that termios understands,
also checks if a given baud rate is defined
for the current system. This check is done during compiling, so
one should already compile with the target architecture in mind
    eg. 9600 -> B9600 (= 00000015)
If baud is an invalid value, the function will return 
BAUDRATE_DEFAULT configured in linklayer.h
*/
speed_t convertBaudRate(int baud);

/*
Calculate a Block Check Character for a given data vector

Return Values
    BCC - BCC was correctly generated
    -1  - somekind of error
*/
u_int8_t generateBCC(u_int8_t *data, int dataSize);

/* 
Check and copy linklayer parameters
Invalid values are given default values assigned in linklayer.h

Return Values
    pointer to new struct
    NULL - error
*/
linkLayer *checkParameters(linkLayer link);

/* 
Write current time and a given string str to a file
Used for logging of events

Return values
    1 - always
*/
int writeEventToFile(FILE *fd, time_t *_TIME, char *str);

#endif