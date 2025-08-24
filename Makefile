CXX = clang++
CXXFLAGS = -Wall -Wextra -std=c++17
TARGET = udpchat
SRC = udpchat.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET) *.o

