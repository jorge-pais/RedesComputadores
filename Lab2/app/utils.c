#include "utils.h"

/* 
    Given an TCP socket, this function will try to read
    an FTP reply, write it to str_out and return the reply code
    str_out must be at least BUFFER_SIZE bytes long
Return values:
    code : success
    -1 : in case of error
*/
int16_t readReply(int socketfd, char** str_out){

    if(socketfd < 0 || str_out == NULL)
        return -1;

    int code/* , readbytes */;
    char buffer[BUFFER_SIZE]; 
    bzero(buffer, BUFFER_SIZE);

    /* readbytes =  */read(socketfd, buffer, BUFFER_SIZE);

    code = atoi(buffer);
    if(code == 421) // Service not available, shuting down control connection
        return -1;

    strncpy(*str_out, buffer, BUFFER_SIZE);

    return code;
}

char* decodePasv(char *cmd, int *port_out){
    
    char* aux = strstr(cmd, "(");
    int portBig, portLittle;
    int ip[4];

    sscanf(aux, "(%d, %d, %d, %d, %d, %d)", &ip[0], &ip[1], &ip[2], &ip[3], &portBig, &portLittle);
    
    *port_out = portBig*256 + portLittle;

    char *out = malloc(28 * sizeof(char));
    sprintf(out, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

    return out;
}

u_int8_t getFileFromFTP(url_t* URL, int fileDescriptor){
    
    int sockfd, passive_sockfd;
    struct sockaddr_in server_addr;

    // Resolve host IP address from hostname
    struct hostent *h;
    if((h = gethostbyname(URL->host)) == NULL){
        fprintf(stderr, "Couldn't resolve host\n");
        return 1;
    }
    char *ip_str = inet_ntoa(*(struct in_addr*) h->h_addr);
    printf("Host IP address: %s\n", ip_str);

    //Code borrowed from clientTCP.c, initialize tcp connection 
    bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip_str);	/*32 bit Internet address network byte ordered*/
	//server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    server_addr.sin_port = htons(DEFAULT_PORT);		/*server TCP port must be network byte ordered */

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        fprintf(stderr, "Error opening network socket\n");
        return 1;
    }

    if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        fprintf(stderr, "Error connecting socket\n");
        return 1;
    }
    //TCP connection established

    // Now for the real FTP connection

    //write(sockfd, "Mensagem de teste na travessia da pilha TCP/IP\n", 48);
    char *buffer = malloc(BUFFER_SIZE * sizeof(char));
    int16_t answer;
    
    usleep(1000*100); //Wait 100ms before trying to reach the server 
    /* for (int i = 0; i < 10; i++)
    {
        bzero(reply, BUFFER_SIZE);
        answer = readReply(sockfd, &reply);
        //printf("%d \n\n", answer);

        for (int i = 0; i < strlen(reply); i++)
            printf("%c", reply[i]);
        printf("----------End of reply----------\n");
    }
     */
    
    if(readReply(sockfd, &buffer) == 120) //Service is not ready yet
        if(readReply(sockfd, &buffer) != 220){ // didnt receive 220
            fprintf(stderr, "Server is showing weird behaviour\n");
            return 1;
        }
    
    sprintf(buffer, "user %s\r\n", URL->username);
    write(sockfd, buffer, strlen(buffer));

    bzero(buffer, BUFFER_SIZE * sizeof(char));
    if(readReply(sockfd, &buffer) != 331){ 
        fprintf(stderr, "Didn't receive 331 reply\n");
        return 1;
    }
    sprintf(buffer, "pass %s\r\n", URL->password);
    write(sockfd, buffer, strlen(buffer));


    bzero(buffer, BUFFER_SIZE * sizeof(char));
    if((answer = readReply(sockfd, &buffer)) != 230){ 
        fprintf(stderr, "wrong username or password\n");
        return 1;
    }
    

    //Open data conenction (passive mode)
    sprintf(buffer, "PASV\r\n");
    write(sockfd, buffer, strlen(buffer));

    readReply(sockfd, &buffer);
    
    int pasvPort;
    char* pasvIP_str = decodePasv(buffer, &pasvPort);

    DEBUG_PRINT("Passive data ip+port %s:%d\n", pasvIP_str, pasvPort);

    // Here we can reuse the server_addr struct for creating the TCP socket 
    bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(pasvIP_str);	/*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(pasvPort);		/*server TCP port must be network byte ordered */

    if((passive_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        fprintf(stderr, "Error opening network socket\n");
        return 1;
    }
    if(connect(passive_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        fprintf(stderr, "Error connecting socket\n");
        return 1;
    }

    //Image/binary, use type A for ASCII
    sprintf(buffer, "TYPE I \r\n"); 
    write(sockfd, buffer, strlen(buffer));

    sprintf(buffer, "RETR %s\r\n", URL->filepath);
    write(sockfd, buffer, strlen(buffer));

    /* This gives us the file size 
    Maybe add a progress bar ?
    */

    readReply(sockfd, &buffer);
    /* if( != 150){
        fprintf(stderr, "File status error, perhaps the file doesn't exist?\n");
        return 1;
    } */

    FILE *downloadedFile = fopen(URL->filename, "w");
    if(downloadedFile == NULL){
        fprintf(stderr, "Error creating file\n");
        return 1;
    }
    int readBytes = 1;
    while (readBytes > 0)
    {
        readBytes = read(passive_sockfd, buffer, BUFFER_SIZE);

        fwrite(buffer, readBytes, 1, downloadedFile);
    }

    fclose(downloadedFile);
    close(passive_sockfd);
    close(sockfd);

    return 0;
}