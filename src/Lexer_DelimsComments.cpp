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
            // Masuk mode komentar (*...*)
            advance(); // konsumsi '('
            advance(); // konsumsi '*'

            std::string body = "(*";
            // DFA: terus baca sampai ketemu '*)'
            while (true) {
                if (currentChar == '\0') {
                    throw std::runtime_error(
                        "Lexical Error at Line " + std::to_string(startLine) +
                        ", Col " + std::to_string(startColumn) +
                        ": Unterminated comment '(*'"
                    );
                }
                if (currentChar == '*' && peek() == ')') {
                    body += '*';
                    advance(); // konsumsi '*'
                    body += ')';
                    advance(); // konsumsi ')'
                    break;
                }
                body += currentChar;
                advance();
            }
            return Token(TokenType::COMMENT, body, startLine, startColumn);
        }

        // Bukan komentar, kembalikan LPARENT
        advance();
        return Token(TokenType::LPARENT, "(", startLine, startColumn);
    }

    // ----------------------------------------------------------------
    // Komentar { ... }
    // ----------------------------------------------------------------
    if (currentChar == '{') {
        advance(); // konsumsi '{'
        std::string body = "{";

        while (true) {
            if (currentChar == '\0') {
                throw std::runtime_error(
                    "Lexical Error at Line " + std::to_string(startLine) +
                    ", Col " + std::to_string(startColumn) +
                    ": Unterminated comment '{'"
                );
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
            throw std::runtime_error(
                "Lexical Error at Line " + std::to_string(startLine) +
                ", Col " + std::to_string(startColumn) +
                ": Unknown character '" + c + "'"
            );
    }
}