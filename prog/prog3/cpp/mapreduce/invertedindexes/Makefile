CXX		= g++
CXXFLAGS	= -O3

all:	mapper reducer

mapper: mapper.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

reducer: reducer.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o mapper reducer
