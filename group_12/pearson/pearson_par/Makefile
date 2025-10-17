# Author: David Holmqvist <daae19@student.bth.se>

CXX=g++-13
# -O3 optimization for the compiler
CXXFLAGS=-std=c++17 -g -Wunused -Wall -Wunused -pg -g -O3 -mavx

all: pearson verify

pearson: vector dataset analysis pearson.cpp 
	$(CXX) $(CXXFLAGS) pearson.cpp vector.o dataset.o analysis.o -o pearson

pearson_par: vector dataset analysis pearson_par.cpp 
	$(CXX) $(CXXFLAGS) pearson_par.cpp vector.o dataset.o analysis.o -o pearson_par

analysis: vector analysis.hpp analysis.cpp
	$(CXX) $(CXXFLAGS) -c analysis.cpp -o analysis.o

dataset: vector dataset.hpp dataset.cpp
	$(CXX) $(CXXFLAGS) -c dataset.cpp -o dataset.o

vector: vector.hpp vector.cpp
	$(CXX) $(CXXFLAGS) -c vector.cpp -o vector.o

verify: verify.c
	$(CC) verify.c -o verify

clean:
	rm -rf verify pearson *.o *.dSYM 2> /dev/null
