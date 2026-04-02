#include "Lexer.hpp"
#include <stdexcept>

Token Lexer::lexDelimiterOrComment() {
    int startLine   = currentLine;
    int startColumn = currentColumn;

    // ----------------------------------------------------------------
    // Kurung kiri: bisa LPARENT atau awal komentar (*...*)
    // ----------------------------------------------------------------
    if (currentChar == '(') {
        if (peek() == '*') {
            advance(); advance(); 

            std::string body = "(*";
            while (true) {
                if (currentChar == '\0' || currentChar == '\n' || currentChar == '\r') {
                    return Token(TokenType::UNKNOWN, "Unterminated comment '(*'", startLine, startColumn);
                }
                if (currentChar == '*' && peek() == ')') {
                    body += '*'; advance(); 
                    body += ')'; advance(); 
                    break;
                }
                body += currentChar;
                advance();
            }
            return Token(TokenType::COMMENT, body, startLine, startColumn);
        }

        advance();
        return Token(TokenType::LPARENT, "(", startLine, startColumn);
    }

    // ----------------------------------------------------------------
    // Komentar { ... }
    // ----------------------------------------------------------------
    if (currentChar == '{') {
        advance();
        std::string body = "{";

        while (true) {
            if (currentChar == '\0' || currentChar == '\n' || currentChar == '\r') {
                return Token(TokenType::UNKNOWN, "Unterminated comment '{'", startLine, startColumn);
            }
            if (currentChar == '}') {
                body += '}';
                advance();
                break;
            }
            body += currentChar;
            advance();
        }
        return Token(TokenType::COMMENT, body, startLine, startColumn);
    }

    // ----------------------------------------------------------------
    // Delimiter satu karakter
    // ----------------------------------------------------------------
    char c = currentChar;
    advance();

    switch (c) {
        case ')': return Token(TokenType::RPARENT,   ")", startLine, startColumn);
        case '[': return Token(TokenType::LBRACK,    "[", startLine, startColumn);
        case ']': return Token(TokenType::RBRACK,    "]", startLine, startColumn);
        case ',': return Token(TokenType::COMMA,     ",", startLine, startColumn);
        case ';': return Token(TokenType::SEMICOLON, ";", startLine, startColumn);
        case '.': return Token(TokenType::PERIOD,    ".", startLine, startColumn);
        default:
            return Token(TokenType::UNKNOWN, "Unknown character '" + std::string(1, c) + "'", startLine, startColumn);
    }
}