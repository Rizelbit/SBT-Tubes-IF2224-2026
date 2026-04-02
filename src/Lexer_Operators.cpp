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
            // State S0: consumed '='
            advance();  // consume '=', look at next char
            if (currentChar == '=') {
                // State S1: '==' → EQL
                advance();
                return Token(TokenType::EQL, "==", startLine, startColumn);
            }
            // No valid follow-up → retract, return UNKNOWN for bare '='
            // Actually bare '=' has no retraction needed — currentChar is
            // already the NEXT char (not consumed into our token).
            return Token(TokenType::UNKNOWN, "=", startLine, startColumn);
        }
            
        case '<': {
            // State S0: consumed '<'
            advance();  // look at next char
            if (currentChar == '>') {
                // '<>' → NEQ
                advance();
                return Token(TokenType::NEQ, "<>", startLine, startColumn);
            } else if (currentChar == '=') {
                // '<=' → LEQ
                advance();
                return Token(TokenType::LEQ, "<=", startLine, startColumn);
            }
            // Just '<' — currentChar is already the next unrelated char
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