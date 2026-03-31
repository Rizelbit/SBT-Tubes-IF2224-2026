#ifndef TOKEN_H
#define TOKEN_H

#include <string>

// Enum Arion (52 Token)
enum class TokenType {
    // Tipe Data & Konstanta
    INTCON, REALCON, CHARCON, STRING,
    
    // Operator Logika & Aritmatika
    NOTSY, PLUS, MINUS, TIMES, IDIV, RDIV, IMOD, ANDSY, ORSY,
    
    // Operator Relasional
    EQL, NEQ, GTR, GEQ, LSS, LEQ,
    
    // Simbol & Tanda Baca
    LPARENT, RPARENT, LBRACK, RBRACK, COMMA, SEMICOLON, PERIOD, COLON, BECOMES,
    
    // Keywords / Reserved Words
    CONSTSY, TYPESY, VARSY, FUNCTIONSY, PROCEDURESY, ARRAYSY, RECORDSY, PROGRAMSY,
    BEGINSY, IFSY, CASESY, REPEATSY, WHILESY, FORSY, ENDSY, ELSESY, UNTILSY, OFSY,
    DOSY, TOSY, DOWNTOSY, THENSY,
    
    // Identifier & Lain-lain
    IDENT, COMMENT,
    
    // Penanda khusus untuk internal lexer
    END_OF_FILE, UNKNOWN
};

class Token {
public:
    TokenType type;
    std::string value; // Lexeme (teks asli dari source code)
    int line;          // Opsional tapi sangat disarankan untuk error handling
    int column;        // Opsional tapi sangat disarankan untuk error handling

    // Constructor
    Token(TokenType type, std::string value, int line = 0, int column = 0);

    // Fungsi untuk mengubah token menjadi string (berguna untuk output file .txt)
    std::string toString() const; 
};

#endif // TOKEN_H