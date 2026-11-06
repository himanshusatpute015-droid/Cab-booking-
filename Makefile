# Build CabGo on Linux / macOS (or Windows with make + MinGW)
CXX      ?= g++
CXXFLAGS ?= -std=c++14 -Wall -Wextra -O2
INCLUDES := -Iinclude
SOURCES  := $(wildcard src/*.cpp src/*/*.cpp)
TARGET   := cabgo

ifeq ($(OS),Windows_NT)
    TARGET := cabgo.exe
    LDLIBS := -lws2_32
endif

$(TARGET): $(SOURCES) $(wildcard include/*/*.h)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SOURCES) -o $@ $(LDLIBS)

run: $(TARGET)
	./$(TARGET) 8080

clean:
	rm -f $(TARGET)

.PHONY: run clean
