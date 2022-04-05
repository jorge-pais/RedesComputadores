#include "linklayer.h"

int main(int argc, char *argv[]){

    if (argc < 3){
		printf("usage: progname /dev/ttySxx tx|rx \n");
		exit(1);
	}

	printf("%s %s\n", argv[1], argv[2]);
	fflush(stdout);

    if(strcmp(argv[2], "tx") == 0){ //tx mode
        printf("tx mode\n");
        struct linkLayer ll;
		sprintf(ll.serialPort, "%s", argv[1]);
		ll.role = TRANSMITTER;
		ll.baudRate = B9600;
		ll.numTries = 3;
		ll.timeOut = 3;

        llopen(ll);

    }
    else if(strcmp(argv[2], "rx") == 0){ //rx mode
        printf("rx mode\n");
        struct linkLayer ll;
		sprintf(ll.serialPort, "%s", argv[1]);
		ll.role = RECEIVER;
		ll.baudRate = B9600;
		ll.numTries = 3;
		ll.timeOut = 3;

        llopen(ll);

    }
    else
        printf("bad parameters\n");

    return 0;
}