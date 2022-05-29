#include "mainApp.h"

u_int8_t parseUrl(char *str, int strLen, url_t *out){

    // ftp://<user>:<password>@<host>/<url-path>
    // ftp://<host>/<url-path>

    char *index, *aux; 
    int len;
    
    //Check for ftp url
    index = strstr(str, "ftp://");
    if(index == NULL) //Invalid url
        return 1; 
    index += 6; // Advance 6 bytes
        
    aux = strstr(index, "@");
    if(aux == NULL){ // There's no user/password 
        sprintf(out->username, "anonymous");
        sprintf(out->password, "password");
    }
    else{
        aux = strstr(index, ":");
        if(aux == NULL)
            return 1;
        len = aux - index;
        if(!len) return 1;
        strncpy(out->username, index, len);
        
        index += len + 1; //advance to password
        
        aux = strstr(index, "@");
        len = aux - index;
        if(!len) return 1;
        strncpy(out->password, index, len);

        index += len + 1;
    }

    aux = strstr(index, "/");
    if(aux == NULL)
        return 1;
    len = aux - index;
    strncpy(out->host, index, len);

    index += len;
    strcpy(out->filepath, index+1); // Our code is now unsafe :)
    
    aux = strstr(index, "/");
    while(aux != NULL){
        index = aux +1;
        aux = strstr(index, "/");
    }

    strcpy(out->filename, index);

    return 0;
}

url_t newUrl(){

    url_t out;
    bzero((char*)&out, sizeof(out));

    return out;
}