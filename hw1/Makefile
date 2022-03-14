CC := gcc
TARGET := hw1
OBJ := hw1.o
CFLAGS := -g -Wall -O3

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm -rf *.o $(TARGET) 
