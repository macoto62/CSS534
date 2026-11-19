CXX		= g++
CXXFLAGS	= -O3

all:	wordcount mapper reducer

wordcount: wordcount.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

mapper: mapper.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

reducer: reducer.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o wordcount mapper reducer all
