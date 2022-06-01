CXX := g++
TARGET := hw4
OBJ := sdb.o hw4.o
CFLAGS := -g -Wall -O3

$(TARGET): $(OBJ)
	$(CXX) $(CFLAGS) $^ -o $@ -lcapstone

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -lcapstone

.PHONY: clean
clean:
	rm -rf *.o $(TARGET)
