CC := gcc
TARGET := hw2
OBJ := hw2.o
CFLAGS := -g -Wall -O3
SHARED := -shared -fPIC
SO := logger.so
TEST := test


all: hw2.c logger.c
	$(CC) -o $(TARGET) $(CFLAGS) hw2.c 
	$(CC) -o $(SO) $(SHARED) logger.c -ldl
	$(CC) -o $(TEST)  $(CFLAGS) test.c

	
.PHONY: clean
clean:
	rm -rf *.o $(TARGET) $(SO) $(TEST) 


