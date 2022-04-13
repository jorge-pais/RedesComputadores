CC = gcc
C_FLAGS = -g -Wall -pipe 

# compile to executable
all:
	$(CC) $(C_FLAGS) *.c -o testApp

# compile to object files
object:
	$(CC) $(C_FLAGS) *.c -c 

clean:
	rm -f testApp a.out
	rm -f *.o