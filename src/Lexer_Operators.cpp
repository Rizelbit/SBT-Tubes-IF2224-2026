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
            
        case '=': {
            advance();
            if (currentChar == '=') {
                advance();
                return Token(TokenType::EQL, "==", startLine, startColumn);
            }
            return Token(TokenType::EQL, "=", startLine, startColumn);
        }
            
        case '<': {
            advance();
            if (currentChar == '>') {
                advance();
                return Token(TokenType::NEQ, "<>", startLine, startColumn);
            } else if (currentChar == '=') {
                advance();
                return Token(TokenType::LEQ, "<=", startLine, startColumn);
            }
            return Token(TokenType::LSS, "<", startLine, startColumn);
        }
            
        case '>': {
            advance();
            if (currentChar == '=') {
                advance();
                return Token(TokenType::GEQ, ">=", startLine, startColumn);
            }
            return Token(TokenType::GTR, ">", startLine, startColumn);
        }
            
        case ':': {
            advance();
            if (currentChar == '=') {
                advance();
                return Token(TokenType::BECOMES, ":=", startLine, startColumn);
            }
            return Token(TokenType::COLON, ":", startLine, startColumn);
        }
            
        default:
            advance();
            return Token(TokenType::UNKNOWN, std::string(1, c), startLine, startColumn);
    }
}