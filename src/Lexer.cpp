#include "Lexer.hpp"
#include <cctype>
#include <stdexcept>

// ============================================================
// Constructor & Destructor
// ============================================================

Lexer::Lexer(const std::string& filename)
    : currentChar('\0'), currentLine(1), currentColumn(0)
{
    file.open(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    advance(); // Baca karakter pertama
}

Lexer::~Lexer() {
    if (file.is_open()) file.close();
}

// ============================================================
// Utilitas Dasar
// ============================================================

void Lexer::advance() {
    if (file.get(currentChar)) {
        if (currentChar == '\n') {
            currentLine++;
            currentColumn = 0;
        } else {
            currentColumn++;
        }
    } else {
        currentChar = '\0'; // EOF sentinel
    }
}

char Lexer::peek() {
    return static_cast<char>(file.peek());
}

void Lexer::skipWhitespace() {
    while (currentChar == ' ' || currentChar == '\t' ||
           currentChar == '\r' || currentChar == '\n') {
        advance();
    }
}

// ============================================================
// getNextToken — dispatcher utama
// ============================================================

Token Lexer::getNextToken() {
    skipWhitespace();

    if (currentChar == '\0') {
        return Token(TokenType::END_OF_FILE, "", currentLine, currentColumn);
    }

    // Identifier / Keyword
    if (std::isalpha(static_cast<unsigned char>(currentChar)) || currentChar == '_') {
        return lexIdentifierOrKeyword();
    }

    // Literal angka, karakter, string
    if (std::isdigit(static_cast<unsigned char>(currentChar)) ||
        currentChar == '\'' || currentChar == '"') {
        return lexLiteral();
    }

    // Delimiter & Komentar: ( ) [ ] , ; . { (* *)
    if (currentChar == '(' || currentChar == ')' ||
        currentChar == '[' || currentChar == ']' ||
        currentChar == ',' || currentChar == ';' ||
        currentChar == '.' || currentChar == '{') {
        return lexDelimiterOrComment();
    }

    // Operator: + - * / = < > :
    if (currentChar == '+' || currentChar == '-' || currentChar == '*' ||
        currentChar == '/' || currentChar == '=' || currentChar == '<' ||
        currentChar == '>' || currentChar == ':') {
        return lexOperator();
    }

    // Karakter ilegal
    int errLine = currentLine;
    int errCol  = currentColumn;
    char bad    = currentChar;
    advance();
    throw std::runtime_error(
        "Lexical Error at Line " + std::to_string(errLine) +
        ", Col " + std::to_string(errCol) +
        ": Unknown character '" + bad + "'"
    );
}

// ============================================================
// tokenizeAll
// ============================================================

std::vector<Token> Lexer::tokenizeAll() {
    std::vector<Token> tokens;
    while (true) {
        Token t = getNextToken();
        tokens.push_back(t);
        if (t.type == TokenType::END_OF_FILE) break;
    }
    return tokens;
}