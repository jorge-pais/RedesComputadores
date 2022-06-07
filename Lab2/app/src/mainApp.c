#include "mainApp.h"

int main(int argc, char *argv[]){

    if(argc != 2){
        fprintf(stderr, "Invalid parameters. Command usage: ./download <FTP_url>\n");
        return 1; // error
    }

    url_t in_url = newUrl();
    if(parseUrl(argv[1], 0, &in_url) != 0){
        fprintf(stderr, "Incorrect URL syntax! Correct format: ftp://user:password@host/file-path\n");
        return 1;
    }
    
    /* printf("username: %s\n", in_url.username);
    printf("password: %s\n", in_url.password);
    printf("host: %s\n", in_url.host);
    printf("file path: %s\n", in_url.filepath); */

    DEBUG_PRINT("Username: %s, len: %ld\n", in_url.username, strlen(in_url.username));
    DEBUG_PRINT("Password: %s, len: %ld\n", in_url.password, strlen(in_url.password));
    DEBUG_PRINT("Host: %s, len: %ld\n", in_url.host, strlen(in_url.host));
    DEBUG_PRINT("File path: %s, len: %ld\n", in_url.filepath, strlen(in_url.filepath));
    DEBUG_PRINT("File name: %s, len: %ld\n", in_url.filename, strlen(in_url.filename));

    return getFileFromFTP(&in_url, 0);

    return 0; 
}