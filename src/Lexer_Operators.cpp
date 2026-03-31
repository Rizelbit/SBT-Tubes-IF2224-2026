#include "Lexer.hpp"

Token Lexer::lexOperator() {
    int startLine = currentLine;
    int startColumn = currentColumn;
    char c = currentChar;

    switch (c) {
        case '+':
            advance();
            return Token(TokenType::PLUS, "+", startLine, startColumn);
            
        case '-':
            advance();
            return Token(TokenType::MINUS, "-", startLine, startColumn);
            
        case '*':
            advance();
            return Token(TokenType::TIMES, "*", startLine, startColumn);
            
        case '/':
            advance();
            return Token(TokenType::RDIV, "/", startLine, startColumn);
            
        case '=':
            if (peek() == '=') {
                advance();
                advance();
                return Token(TokenType::EQL, "==", startLine, startColumn);
            }
            advance();
            return Token(TokenType::UNKNOWN, "=", startLine, startColumn);
            
        case '<':
            if (peek() == '>') {
                advance();
                advance();
                return Token(TokenType::NEQ, "<>", startLine, startColumn);
            } else if (peek() == '=') {
                advance();
                advance();
                return Token(TokenType::LEQ, "<=", startLine, startColumn);
            }
            advance();
            return Token(TokenType::LSS, "<", startLine, startColumn);
            
        case '>':
            if (peek() == '=') {
                advance();
                advance();
                return Token(TokenType::GEQ, ">=", startLine, startColumn);
            }
            advance();
            return Token(TokenType::GTR, ">", startLine, startColumn);
            
        case ':':
            if (peek() == '=') {
                advance();
                advance();
                return Token(TokenType::BECOMES, ":=", startLine, startColumn);
            }
            advance();
            return Token(TokenType::COLON, ":", startLine, startColumn);
            
        default:
            advance();
            return Token(TokenType::UNKNOWN, std::string(1, c), startLine, startColumn);
    }
}