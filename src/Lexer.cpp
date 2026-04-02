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
// Utilitas Dasar — Pure DFA (no peek)
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

void Lexer::retract() {
    if (currentChar == '\0') {
        // At EOF — putback a dummy to rewind the stream, then re-read
        file.clear();  // clear eofbit so putback/seekg works
    }
    file.putback(currentChar);

    if (currentChar == '\n') {
        currentLine--;
        // Column is approximate after retract over newline — but we never
        // retract over a newline in practice (longest-match always decides
        // before a newline boundary).  Set to 0 as a safe fallback.
        currentColumn = 0;
    } else {
        currentColumn--;
    }
    currentChar = '\0'; // invalidate — caller must advance() again to get valid char
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
    // 2. Cek unary minus
    //    '-' followed by digit: advance to consume '-', then advance to see
    //    if next is digit.  If not a valid unary context, retract and treat
    //    as operator.
    else if (currentChar == '-') {
        bool isUnary = (prevTokenType != TokenType::IDENT &&
                        prevTokenType != TokenType::INTCON &&
                        prevTokenType != TokenType::REALCON &&
                        prevTokenType != TokenType::RPARENT &&
                        prevTokenType != TokenType::RBRACK);
        if (isUnary) {
            // Save position, consume '-'
            int saveLine = currentLine;
            int saveCol  = currentColumn;
            char saveCh  = currentChar;
            advance(); // consume '-'
            if (std::isdigit(static_cast<unsigned char>(currentChar))) {
                // It IS a unary minus followed by digit → retract the digit,
                // put '-' back as currentChar, and call lexLiteral which
                // expects currentChar == '-'.
                retract();            // put digit back
                currentChar = saveCh; // restore '-' as currentChar
                // But we already consumed '-' from the stream with the first
                // advance, and then we retracted the digit.  We need the
                // stream positioned right AFTER the '-' so that when
                // lexLiteral does advance() it gets the digit.
                // Actually: retract() put the digit back.  The '-' was
                // consumed by the first advance().  So the stream is now:
                //   stream → [digit] [rest...]
                // We set currentChar = '-' so lexLiteral sees '-' and
                // on its advance() it will get the digit.  This is correct.
                //
                // We also need to restore line/col to the '-' position.
                currentLine = saveLine;
                currentColumn = saveCol;
                result = lexLiteral();
            } else {
                // Not a digit after '-', so '-' is an operator.
                // Retract whatever we just read, restore '-'.
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
    // 3. '+' is always an operator (per parity rule)
    else if (currentChar == '+') {
        result = lexOperator();
    }
    // 4. Literal angka, karakter, string
    else if (std::isdigit(static_cast<unsigned char>(currentChar)) ||
             currentChar == '\'' || currentChar == '"') {
        result = lexLiteral();
    }
    // 5. Delimiter & Komentar
    else if (currentChar == '(' || currentChar == ')' ||
             currentChar == '[' || currentChar == ']' ||
             currentChar == ',' || currentChar == ';' ||
             currentChar == '.' || currentChar == '{') {
        result = lexDelimiterOrComment();
    }
    // 6. Operator
    else if (currentChar == '*' || currentChar == '/' ||
             currentChar == '=' || currentChar == '<' ||
             currentChar == '>' || currentChar == ':') {
        result = lexOperator();
    }
    // 7. Karakter ilegal
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