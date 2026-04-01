CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

SRC = src/main.cpp src/Lexer.cpp src/Token.cpp \
      src/Lexer_DelimsComments.cpp \
      src/Lexer_Keywords.cpp \
      src/Lexer_Literals.cpp \
      src/Lexer_Operators.cpp

OBJ = $(SRC:.cpp=.o)

lexer: $(OBJ)
	$(CXX) $(CXXFLAGS) -o lexer $(OBJ)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: lexer
	./lexer

clean:
	rm -f $(OBJ) lexer