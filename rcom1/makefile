CC = gcc
C_FLAGS = -g -Wall -pipe 

# compile to executable
all:
	$(CC) $(C_FLAGS) *.c -o testApp

# compile to object files
object:
	$(CC) $(C_FLAGS) linklayer.c receiver.c utils.c transmitter.c -c
	cp *.o protocol/ 
	rm -f *.o

clean:
	rm -f testApp a.out application/main
	rm -f *.o protocol/*.o