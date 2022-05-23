CC = gcc
C_FLAGS = -g -Wall -pipe

# compile to object files and compile along side main.c
# also copy linklayer.h, to prevent having two different definitions
# for the API and main() function
all:
	$(CC) $(C_FLAGS) linklayer.c receiver.c utils.c transmitter.c -c
	cp *.o protocol/ && rm -f *.o
	cp -f linklayer.h application/linklayer.h 
	cd application && $(CC) $(C_FLAGS) main.c ../protocol/*.o -o main

# compile to executable using test.c
test:
	$(CC) $(C_FLAGS) *.c -o testApp

clean:
	rm -f testApp a.out application/main
	rm -f application/linklayer.h
	rm -f *.o protocol/*.o