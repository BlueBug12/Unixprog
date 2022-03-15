CXX := g++
TARGET := hw1
OBJ := hw1.o
CFLAGS := -g -Wall -O3

$(TARGET): $(OBJ)
	$(CXX) $(CFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm -rf *.o $(TARGET) 
