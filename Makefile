CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

all: test

test: tests.cpp
	$(CXX) $(CXXFLAGS) -o tests tests.cpp engine.cpp
	./tests

build-submission: engine.cpp
	$(CXX) $(CXXFLAGS) -c engine.cpp -o engine.o
	$(CXX) $(CXXFLAGS) -shared -o engine.so engine.o

clean:
	rm -f tests

