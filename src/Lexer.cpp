#include "Lexer.hpp"
#include <cctype>

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

Token Lexer::getNextToken() {
    skipWhitespace();

    if (currentChar == '\0') {
        return Token(TokenType::END_OF_FILE, "", currentLine, currentColumn);
    }

    Token result(TokenType::UNKNOWN, "", 0, 0);

    // 1. Identifier / Keyword
    if (std::isalpha(static_cast<unsigned char>(currentChar)) || currentChar == '_') {
        result = lexIdentifierOrKeyword();
    }
    // 2. Cek unary minus / plus
    else if ((currentChar == '+' || currentChar == '-') && std::isdigit(static_cast<unsigned char>(peek()))) {
        if (currentChar == '-') {
            bool isUnary = (prevTokenType != TokenType::IDENT &&
                            prevTokenType != TokenType::INTCON &&
                            prevTokenType != TokenType::REALCON &&
                            prevTokenType != TokenType::RPARENT &&
                            prevTokenType != TokenType::RBRACK);
            
            if (isUnary) {
                result = lexLiteral();
            } else {
                result = lexOperator();
            }
        } else {
            result = lexOperator();
        }
    }
    // 3. Literal angka, karakter, string
    else if (std::isdigit(static_cast<unsigned char>(currentChar)) ||
        currentChar == '\'' || currentChar == '"') {
        result = lexLiteral();
    }
    // 4. Delimiter & Komentar
    else if (currentChar == '(' || currentChar == ')' ||
        currentChar == '[' || currentChar == ']' ||
        currentChar == ',' || currentChar == ';' ||
        currentChar == '.' || currentChar == '{') {
        result = lexDelimiterOrComment();
    }
    // 5. Operator
    else if (currentChar == '+' || currentChar == '-' || currentChar == '*' ||
        currentChar == '/' || currentChar == '=' || currentChar == '<' ||
        currentChar == '>' || currentChar == ':') {
        result = lexOperator();
    }
    // 6. Karakter ilegal
    else {
        int errLine = currentLine;
        int errCol  = currentColumn;
        char bad    = currentChar;
        advance();
        result = Token(TokenType::UNKNOWN, "Unknown character '" + std::string(1, bad) + "'", errLine, errCol);
    }

    if (result.type != TokenType::COMMENT && result.type != TokenType::UNKNOWN) {
        prevTokenType = result.type;
    }

    return result;
}

std::vector<Token> Lexer::tokenizeAll() {
    std::vector<Token> tokens;
    while (true) {
        Token t = getNextToken();
        tokens.push_back(t);
        if (t.type == TokenType::END_OF_FILE) break;
    }
    return tokens;
}