CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
TARGET = PointOnLineParam
SOURCE = PointOnLineParam.cpp

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE) -lm

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run