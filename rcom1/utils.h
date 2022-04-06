#ifndef UTILS
#define UTILS

#include "linklayer.h"
#include "definitions.h"

/*  
Globally declared termios structures, and serial terminal 
file descriptor
*/
static struct termios oldtio, newtio;

int configureSerialterminal(linkLayer connectionParameters);
int closeSerialterminal(int fd);
int getCommand(int fd, unsigned char *cmd, int cmdLen);
int getInfoCommand(int fd, unsigned char *cmd, int cmdLen);

//transmitter stuff
int transmitter_llopen(linkLayer connectionParameters);
int transmitter_llclose(linkLayer connectionParameters);
void timeOut();

//receiver stuff
int receiver_llopen(linkLayer connectionParameters);
int receiver_llclose(linkLayer connectionParameters);

#endif