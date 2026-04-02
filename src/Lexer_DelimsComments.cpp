#include "Lexer.hpp"

Token Lexer::lexDelimiterOrComment() {
    int startLine   = currentLine;
    int startColumn = currentColumn;

    if (currentChar == '(') {
        advance();

        if (currentChar == '*') {
            advance();

            std::string body = "(*";

            bool inStar = false;

            while (true) {
                if (currentChar == '\0') {
                    return Token(TokenType::UNKNOWN, "Unterminated comment '(*'", startLine, startColumn);
                }

                if (inStar) {
                    if (currentChar == ')') {
                        body += ')';
                        advance();
                        break;
                    }
                    inStar = false;
                }

                if (currentChar == '*') {
                    inStar = true;
                    body += currentChar;
                    advance();
                } else {
                    body += currentChar;
                    advance();
                }
            }

            return Token(TokenType::COMMENT, body, startLine, startColumn);
        }

        return Token(TokenType::LPARENT, "(", startLine, startColumn);
    }

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