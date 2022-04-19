CC = gcc
C_FLAGS = -g -Wall -pipe 

# compile to object files and run build
all:
	$(CC) $(C_FLAGS) linklayer.c receiver.c utils.c transmitter.c -c
	cp *.o protocol/ && rm -f *.o
	cd application && ./build.sh 

# compile to executable using test.c
test:
	$(CC) $(C_FLAGS) *.c -o testApp

clean:
	rm -f testApp a.out application/main
	rm -f *.o protocol/*.o