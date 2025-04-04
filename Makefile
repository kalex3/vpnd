CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra
LDFLAGS = -lemon

all: algorithm1 algorithm2 erdos-renyi lower_bound

algorithm1: algorithm1.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

algorithm2: algorithm2.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

erdos-renyi: erdos-renyi.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

lower_bound: lower_bound.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f algorithm1 algorithm2 erdos-renyi lower_bound

.PHONY: all clean 
