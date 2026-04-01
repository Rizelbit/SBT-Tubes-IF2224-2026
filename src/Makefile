CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2
TARGET   := lexer

SRCS := main.cpp \
        Token.cpp \
        Lexer.cpp \
        Lexer_DelimsComments.cpp \
        Lexer_Keywords.cpp \
        Lexer_Literals.cpp \
        Lexer_Operators.cpp

OBJS := $(SRCS:.cpp=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

run: all
	./$(TARGET) input.txt output.txt

clean:
	rm -f $(OBJS) $(TARGET)