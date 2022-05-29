#ifndef UTILS_H
#define UTILS_H

#include "defines.h"
#include "parseUrl.h"
#include "mainApp.h"

u_int8_t getFileFromFTP(url_t* URL, int fileDescriptor);

#endif