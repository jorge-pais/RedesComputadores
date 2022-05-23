#include "mainApp.h"

// Testing with netlab ftp server
#define SERVER_ADDR "192.168.109.136"
#define SERVER_PORT "21" // default ftp port

int main(int argc, char *argv[]){

    if(argc != 2){
        printf("Invalid parameters. Correct syntax ./download <FTP_url>\n");
        return 1; // error
    }

    url_t test;

    if(parseUrl(argv[1], 0, &test) != 0){
        printf("Incorrect URL syntax! Correct format: ftp://user:password@host/file-path\n");
        return 1;
    }
    
    printf("username: %s\n", test.username);
    printf("password: %s\n", test.password);
    printf("host: %s\n", test.host);
    printf("file path: %s\n", test.filepath);

    int sockfd;
    struct sockaddr_in server_addr;

    return 0; 
}