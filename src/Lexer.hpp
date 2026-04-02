#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <fstream>
#include <vector>
#include "Token.hpp"

class Lexer {
private:
    std::ifstream file;
    char currentChar;
    int currentLine;
    int currentColumn;

    // ====================================================================
    // FUNGSI UTILITAS DASAR (Pure DFA — no peek())
    // ====================================================================
    void advance();
    void retract();
    void skipWhitespace();

    TokenType prevTokenType = TokenType::UNKNOWN;

    // ====================================================================
    // FUNGSI PEMBANTU DFA
    // ====================================================================
    
    Token lexLiteral(); 

    Token lexOperator(); 

    Token lexIdentifierOrKeyword(); 

    Token lexDelimiterOrComment(); 

public:
    Lexer(const std::string& filename);
    
    ~Lexer();

    Token getNextToken();

    std::vector<Token> tokenizeAll();
};

#endif