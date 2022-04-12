#include "linklayer.h"
#include "transmitter.h"

int main(int argc, char *argv[]){

    if (argc < 3){
		printf("usage: progname /dev/ttySxx tx|rx \n");
		exit(1);
	}

    //test stuffing
    /* u_int8_t data[6] = {0x00, 0x01, 0x7E, 0x44, 0x7D, 0x74};
    int size = 0;

    u_int8_t *newData = prepareInfoFrame(data, 6, &size, 1);

    for (int i = 0; i < size; i++)
        printf("0x%02x ", newData[i]);
    
    printf("\n"); */

	printf("%s %s\n", argv[1], argv[2]);
	fflush(stdout);

	struct linkLayer ll;
		sprintf(ll.serialPort, "%s", argv[1]);
		ll.baudRate = 9600;
		ll.numTries = 3;
		ll.timeOut = 3;

    if(strcmp(argv[2], "tx") == 0){ //tx mode
        printf("tx mode\n");
		ll.role = TRANSMITTER;
        
        llopen(ll);
        char text[] = "Olá netedu!";
        llwrite(text, 13);
    }
    else if(strcmp(argv[2], "rx") == 0){ //rx mode
        printf("rx mode\n");
		ll.role = RECEIVER;
        
        llopen(ll);

        char *text;
        llread(text);
    }
    else
        printf("bad parameters\n");

    return 0;
}