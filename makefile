# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

# Targets
all: TextualTide

TextualTide: main.o
	$(CXX) $(CXXFLAGS) -o $@ $^

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f *.o TextualTide

run: TextualTide
	./TextualTide