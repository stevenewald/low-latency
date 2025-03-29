CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

all: test

test: tests.cpp
	$(CXX) $(CXXFLAGS) -o tests tests.cpp engine.cpp
	./tests

clean:
	rm -f tests

