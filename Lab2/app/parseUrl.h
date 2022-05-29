#ifndef PARSEURL_H
#define PARSEURL_H

#include "defines.h"
#include "mainApp.h"

typedef struct url_t_
{
    char username [NAME_SIZE];
    char password [NAME_SIZE];
    char host [NAME_SIZE];
    char filepath [PATH_SIZE];
    char filename [NAME_SIZE];
} url_t;

/* 
    Given a valid url string, identify all components
    which constitute it. 
Return Values:
    0- success
    1- invalid string or error
*/  
u_int8_t parseUrl(char *str, int strLen, url_t *out);

/* 
    Creates a new url_t structure, zero'd out.
*/
url_t newUrl();


#endif