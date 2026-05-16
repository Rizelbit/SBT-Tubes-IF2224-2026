CXX      := g++
# Tambahkan $(SRCDIR) setelah -I agar header terbaca
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -Isrc
TARGET   := arion
SRCDIR   := src
TESTDIR  := test/milestone-3

# Daftar semua file sumber
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
        $(SRCDIR)/ASTPrinter.cpp 

OBJS := $(SRCS:.cpp=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(SRCDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

run: all
	./$(TARGET) $(TESTDIR)/input1.txt $(TESTDIR)/output1.txt

clean:
	rm -f $(OBJS) $(TARGET)
