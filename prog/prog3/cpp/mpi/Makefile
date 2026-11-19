CXX		= mpic++

all:	invertedindexes

invertedindexes: invertedindexes.cpp Timer.cpp
	$(CXX) -o $@ $^

%.o: %.cpp
	$(CXX) -c $< -o $@

clean:
	rm -f *.o invertedindexes
