#ifndef DEFINITIONS_H
#define DEFINITIONS_H

//Frame formats
#define FLAG        0x7E    //0b01111110
#define ESC         0x7D    //0b01111101
//Address fields
#define A_tx        0x03    //Commands sent by the Transmitter and Answers from the receiver
#define A_rx        0x01    //Commands sent by the Receiver and Answers from the transmitter
//Supervision and Unnumbered Frame Control
#define C_SET       0x03    //0b00000011
#define C_DISC      0x0B    //0b00001011
#define C_UA        0x07    //0b00000111
#define C_RR(R)     ((R<<5) + 1)  //R = Nr
#define C_REJ(R)    ((R<<5) + 5)
#define SU_SEQ(C)   ((C & 0b00100000) >> 5)
//Information frame control
#define C(S)        (S<<1)      //S = Ns

#define DEBUG
#ifdef DEBUG
    #define DEBUG_PRINT(str, ...) printf(str, ##__VA_ARGS__)
#else
    #define DEBUG_PRINT(str, ...)
#endif

#endif