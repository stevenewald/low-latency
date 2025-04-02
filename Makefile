CXX = g++

BOOST_INCLUDE = /nix/store/hby0y11ppws9znf05x07k9apf84ac6bl-boost-1.81.0-dev/include
BOOST_LIB = /nix/store/5273afwl2bjhy47kmhhv9vnx1bv76l5a-boost-1.81.0/lib

CXXFLAGS = -std=c++20 -Wall -Wextra -O3 -march=native 
CXXFLAGS += -flto -mtune=native -funroll-loops -ffast-math -fomit-frame-pointer -fprefetch-loop-arrays -falign-functions=32
CXXFLAGS += -falign-loops=32 -fno-stack-protector -fno-math-errno -fstrict-aliasing -fno-semantic-interposition -finline-functions -finline-limit=1000 -fipa-pta
CXXFLAGS += -I$(BOOST_INCLUDE)

LDFLAGS += -L$(BOOST_LIB)
LDLIBS += -lboost_system -lboost_filesystem

MAKEFILE_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

all: test

test: tests.cpp
	$(CXX) $(CXXFLAGS) -o tests tests.cpp engine.cpp $(LDFLAGS) $(LDLIBS)
	./tests

submit: engine.cpp
	$(CXX) $(CXXFLAGS) -fPIC -c engine.cpp -o engine.o
	$(CXX) $(CXXFLAGS) -shared -o engine.so engine.o $(LDFLAGS) $(LDLIBS)
	lll-bench $(MAKEFILE_DIR)engine.so

clean:
	rm -f tests engine.o engine.so
