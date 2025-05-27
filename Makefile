CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra
LDFLAGS = -lemon

all: main erdos-renyi

main: main.cpp vpnd.h vpnd.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

erdos-renyi: erdos-renyi.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f main erdos-renyi

.PHONY: all clean
