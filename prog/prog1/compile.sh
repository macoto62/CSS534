#!/bin/sh

g++ initialize.cpp -o initialize
g++ -c EvalXOverMutate.cpp -fopenmp
g++ -c Timer.cpp
g++ Tsp.cpp Timer.o EvalXOverMutate.o -fopenmp -o Tsp

# CXXFLAGS="-O2"
# g++ $CXXFLAGS initialize.cpp -o initialize
# g++ $CXXFLAGS -c EvalXOverMutate.cpp -fopenmp
# g++ $CXXFLAGS -c Timer.cpp
# g++ $CXXFLAGS Tsp.cpp Timer.o EvalXOverMutate.o -fopenmp -o Tsp

# CXXFLAGS="-pg"
# g++ $CXXFLAGS initialize.cpp -o initialize
# g++ $CXXFLAGS -c EvalXOverMutate.cpp -fopenmp
# g++ $CXXFLAGS -c Timer.cpp
# g++ $CXXFLAGS Tsp.cpp Timer.o EvalXOverMutate.o -fopenmp -o Tsp
