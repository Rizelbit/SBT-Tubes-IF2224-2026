CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -Isrc
TARGET   := arion
SRCDIR   := src
TESTDIR  := test/milestone-3

SRCS := $(SRCDIR)/Main.cpp \
        $(SRCDIR)/Token.cpp \
        $(SRCDIR)/Lexer.cpp \
        $(SRCDIR)/Lexer_DelimsComments.cpp \
        $(SRCDIR)/Lexer_Keywords.cpp \
        $(SRCDIR)/Lexer_Literals.cpp \
        $(SRCDIR)/Lexer_Operators.cpp \
        $(SRCDIR)/Parser.cpp \
        $(SRCDIR)/Parser_Core.cpp \
        $(SRCDIR)/Parser_Declarations.cpp \
        $(SRCDIR)/Parser_ControlFlow.cpp \
        $(SRCDIR)/Parser_Expression.cpp \
        $(SRCDIR)/ParseNode.cpp \
        $(SRCDIR)/AST.cpp \
        $(SRCDIR)/ASTBuilder.cpp \
        $(SRCDIR)/ASTPrinter.cpp \
        $(SRCDIR)/TypeSystem.cpp \
        $(SRCDIR)/SymbolTable.cpp \
        $(SRCDIR)/SemanticAnalyzer.cpp

OBJS := $(SRCS:.cpp=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(SRCDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

run: all
	./$(TARGET) $(TESTDIR)/input3.txt $(TESTDIR)/output3.txt

clean:
	rm -f $(OBJS) $(TARGET)