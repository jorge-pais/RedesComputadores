#ifndef DEFINES_H
#define DEFINES_H

#define DEFAULT_PORT 21

#define NAME_SIZE 256
#define PATH_SIZE 1024
#define BUFFER_SIZE 2048

#define DEBUG

#ifdef DEBUG
    #define DEBUG_PRINT(str, ...) (printf("[DEBUG] "), printf(str, ##__VA_ARGS__))
#else
    #define DEBUG_PRINT(str, ...)
#endif

#endif

/* for (int i = 0; i < strlen(buffer); i++)
        printf("%c", buffer[i]); */