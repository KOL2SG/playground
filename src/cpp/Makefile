# Simple Makefile for single-file C++ program

CXX = g++
CXXFLAGS = -g -Wall -std=c++17

TARGET = palindrome_Linklist
SRC = palindrome_Linklist.cpp
build: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: build
	./$(TARGET)

clean:
	rm -f $(TARGET)
