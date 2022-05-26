CXX := g++
TARGET := hw4
OBJ := sdb.o hw4.o
CFLAGS := -g -Wall -O3

$(TARGET): $(OBJ)
	$(CXX) $(CFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm -rf *.o $(TARGET)
