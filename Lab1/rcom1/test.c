#include "linklayer.h"
#include "transmitter.h"
#include "receiver.h"

int main(int argc, char *argv[]){

    if (argc < 3){
		printf("usage: progname /dev/ttySxx tx|rx \n");
		exit(1);
	}

    //test stuffing
    //stuffingTests();

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
        
        if (llopen(ll) == 1){
            char text[] = "Ola netedu!";
            llwrite(text, 12);
        }
    }
    else if(strcmp(argv[2], "rx") == 0){ //rx mode
        printf("rx mode\n");
		ll.role = RECEIVER;

        if(llopen(ll)==0){
            char text[MAX_PAYLOAD_SIZE];
            llread(text);
        }
    }
    else
        printf("bad parameters\n");

    return 0;
}

void stuffingTests(){

    u_int8_t data[6] = {0x00, 0x01, 0x7E, 0x44, 0x7D, 0x74};
    u_int8_t *newData;
    int size = 0;

    newData = prepareInfoFrame(data, 6, &size, 1);
    for (int i = 0; i < size; i++)
        printf("0x%02x ", newData[i]);
    printf("\n");

    newData = byteStuffing(data, 6, &size);
    for (int i = 0; i < size; i++)
        printf("0x%02x ", newData[i]);
    printf("\n");

    newData = byteDestuffing(newData, size, &size);
    for (int i = 0; i < size; i++)
        printf("0x%02x ", newData[i]);
    printf("\n");

    return;
}