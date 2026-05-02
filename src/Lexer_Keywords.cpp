#include "Lexer.hpp"
#include <unordered_map>
#include <cctype>
#include <algorithm>

Token Lexer::lexIdentifierOrKeyword() {
    std::string lexeme = "";
    
    // Menyimpan posisi awal token untuk keperluan pelacakan
    int startLine = currentLine;
    int startColumn = currentColumn;

    // biar bisa variabel snake_case
    while (std::isalnum(static_cast<unsigned char>(currentChar)) || currentChar == '_') {
        lexeme += currentChar;
        advance();
    }

    std::string lowerLexeme = lexeme;
    std::transform(lowerLexeme.begin(), lowerLexeme.end(), lowerLexeme.begin(), ::tolower);

    static const std::unordered_map<std::string, TokenType> keywordTable = {
        // Deklarasi
        {"const", TokenType::CONSTSY},
        {"type", TokenType::TYPESY},
        {"var", TokenType::VARSY},
        {"function", TokenType::FUNCTIONSY},
        {"procedure", TokenType::PROCEDURESY},
        {"array", TokenType::ARRAYSY},
        {"record", TokenType::RECORDSY},
        {"program", TokenType::PROGRAMSY},

        // Keyword Control Flow
        {"begin", TokenType::BEGINSY},
        {"if", TokenType::IFSY},
        {"case", TokenType::CASESY},
        {"repeat", TokenType::REPEATSY},
        {"while", TokenType::WHILESY},
        {"for", TokenType::FORSY},
        {"end", TokenType::ENDSY},
        {"else", TokenType::ELSESY},
        {"until", TokenType::UNTILSY},
        {"of", TokenType::OFSY},
        {"do", TokenType::DOSY},
        {"to", TokenType::TOSY},
        {"downto", TokenType::DOWNTOSY},
        {"then", TokenType::THENSY},

        // Text-based Operators
        {"not", TokenType::NOTSY},
        {"and", TokenType::ANDSY},
        {"or", TokenType::ORSY},
        {"div", TokenType::IDIV},
        {"mod", TokenType::IMOD}
    };

    auto it = keywordTable.find(lowerLexeme);
    if (it != keywordTable.end()) {
        return Token(it->second, lexeme, startLine, startColumn);
    }

    return Token(TokenType::IDENT, lexeme, startLine, startColumn);
}