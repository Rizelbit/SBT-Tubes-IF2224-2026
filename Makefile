CXX      := g++
# Tambahkan $(SRCDIR) setelah -I agar header terbaca
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -Isrc
TARGET   := lexer
SRCDIR   := src
TESTDIR  := test/milestone-1

# Daftar semua file sumber
SRCS := $(SRCDIR)/Main.cpp \
        $(SRCDIR)/Token.cpp \
        $(SRCDIR)/Lexer.cpp \
        $(SRCDIR)/Lexer_DelimsComments.cpp \
        $(SRCDIR)/Lexer_Keywords.cpp \
        $(SRCDIR)/Lexer_Literals.cpp \
        $(SRCDIR)/Lexer_Operators.cpp

OBJS := $(SRCS:.cpp=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(SRCDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

run: all
	./$(TARGET) $(TESTDIR)/test1.txt $(TESTDIR)/output.txt

clean:
	rm -f $(OBJS) $(TARGET)