CXX := g++
CC := gcc
CXXFLAGS := -Iinclude -std=c++17 -Wall -Wextra -pedantic
CFLAGS := -Iinclude -Wall -Wextra -pedantic
LDFLAGS := -lglfw -lGL -ldl

CPP_SOURCES := $(wildcard src/*.cpp)
CPP_OBJECTS := $(CPP_SOURCES:.cpp=.o)
C_OBJECTS := src/gl.o
OBJECTS := $(CPP_OBJECTS) $(C_OBJECTS)

e exe: main.exe

main.exe: $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

src/gl.o: src/gl.c
	$(CC) $(CFLAGS) -c $< -o $@

r run: main.exe
	./main.exe

c clean:
	rm -f src/*.o main.exe
