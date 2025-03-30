CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

all: test

test: tests.cpp
	$(CXX) $(CXXFLAGS) -o tests tests.cpp engine.cpp
	./tests

submit: engine.cpp
	$(CXX) $(CXXFLAGS) -fPIC -c engine.cpp -o engine.o
	$(CXX) $(CXXFLAGS) -shared -o engine.so engine.o
	lll-bench # Runs the submission binary. If this errors, contact Steve

clean:
	rm -f tests engine.o engine.so
