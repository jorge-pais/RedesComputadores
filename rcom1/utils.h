#ifndef UTILS
#define UTILS

#include "linklayer.h"
#include "definitions.h"

int configureSerialterminal(linkLayer connectionParameters);
int getCommand(int fd, unsigned char *cmd, int cmdLen);


#endif