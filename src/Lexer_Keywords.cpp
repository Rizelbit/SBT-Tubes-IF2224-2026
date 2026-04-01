#include "Lexer.hpp"
#include <unordered_map>
#include <cctype>
#include <algorithm>

Token Lexer::lexIdentifierOrKeyword() {
    std::string lexeme = "";
    
    // Menyimpan posisi awal token untuk keperluan pelacakan
    int startLine = currentLine;
    int startColumn = currentColumn;

    // State DFA: Terus membaca selama karakter saat ini adalah huruf atau angka 
    while (std::isalnum(currentChar)) {
        lexeme += currentChar;
        advance(); // Maju ke karakter berikutnya di file
    }

    // Ubah salinan lexeme menjadi lowercase
    std::string lowerLexeme = lexeme;
    std::transform(lowerLexeme.begin(), lowerLexeme.end(), lowerLexeme.begin(), ::tolower);

    // Lookup Table statis
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

    // Cari string lowercase tersebut di dalam Lookup Table
    auto it = keywordTable.find(lowerLexeme);
    if (it != keywordTable.end()) {
        return Token(it->second, lexeme, startLine, startColumn);
    }

    return Token(TokenType::IDENT, lexeme, startLine, startColumn);
}