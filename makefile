# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
DOCTEST_FLAGS = -DDOCTEST_CONFIG_IMPLEMENT

# Targets
all: TextualTide TextualTideTests

TextualTide: main.o
	$(CXX) $(CXXFLAGS) -o $@ $^

TextualTideTests: tests.o
	$(CXX) $(CXXFLAGS) -o $@ $^

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c $<

tests.o: tests.cpp
	$(CXX) $(CXXFLAGS) $(DOCTEST_FLAGS) -c $<

clean:
	rm -f *.o TextualTide TextualTideTests

run: TextualTide
	./TextualTide

test: TextualTideTests
	./TextualTideTests