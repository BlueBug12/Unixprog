CXX := g++
TARGET := hw4
OBJ := sdb.o hw4.o
CFLAGS := -g -Wall -O3 -lcapstone
INCLUDE = /usr/lib/

$(TARGET): $(OBJ)
	$(CXX) $(CFLAGS) $^ -o $@ -I$(INCLUDE)

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -I$(INCLUDE)

.PHONY: clean
clean:
	rm -rf *.o $(TARGET)
