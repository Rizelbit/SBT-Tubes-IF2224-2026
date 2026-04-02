#include "Lexer.hpp"
#include <cctype>


Lexer::Lexer(const std::string& filename)
    : currentChar('\0'), currentLine(1), currentColumn(0)
{
    file.open(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    advance();
}

Lexer::~Lexer() {
    if (file.is_open()) file.close();
}


void Lexer::advance() {
    if (file.get(currentChar)) {
        if (currentChar == '\n') {
            currentLine++;
            currentColumn = 0;
        } else {
            currentColumn++;
        }
    } else {
        currentChar = '\0';
    }
}

void Lexer::retract() {
    if (currentChar == '\0') {
        file.clear();
    }
    file.putback(currentChar);

    if (currentChar == '\n') {
        currentLine--;
        currentColumn = 0;
    } else {
        currentColumn--;
    }
    currentChar = '\0';
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

    if (std::isalpha(static_cast<unsigned char>(currentChar)) || currentChar == '_') {
        result = lexIdentifierOrKeyword();
    }
    else if (currentChar == '-') {
        bool isUnary = (prevTokenType != TokenType::IDENT &&
                        prevTokenType != TokenType::INTCON &&
                        prevTokenType != TokenType::REALCON &&
                        prevTokenType != TokenType::RPARENT &&
                        prevTokenType != TokenType::RBRACK);
        if (isUnary) {
            int saveLine = currentLine;
            int saveCol  = currentColumn;
            char saveCh  = currentChar;
            advance();
            if (std::isdigit(static_cast<unsigned char>(currentChar))) {
                retract();
                currentChar = saveCh;
                currentLine = saveLine;
                currentColumn = saveCol;
                result = lexLiteral();
            } else {
                retract();
                currentChar = saveCh;
                currentLine = saveLine;
                currentColumn = saveCol;
                result = lexOperator();
            }
        } else {
            result = lexOperator();
        }
    }
    else if (currentChar == '+') {
        result = lexOperator();
    }
    else if (std::isdigit(static_cast<unsigned char>(currentChar)) ||
             currentChar == '\'' || currentChar == '"') {
        result = lexLiteral();
    }
    else if (currentChar == '(' || currentChar == ')' ||
             currentChar == '[' || currentChar == ']' ||
             currentChar == ',' || currentChar == ';' ||
             currentChar == '.' || currentChar == '{') {
        result = lexDelimiterOrComment();
    }
    else if (currentChar == '*' || currentChar == '/' ||
             currentChar == '=' || currentChar == '<' ||
             currentChar == '>' || currentChar == ':') {
        result = lexOperator();
    }
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