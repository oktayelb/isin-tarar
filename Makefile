CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Wextra

SOURCES = $(wildcard *.cpp)
HEADERS = $(wildcard *.h)

raytracer: $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o raytracer $(SOURCES)

clean:
	rm -f raytracer

.PHONY: clean
