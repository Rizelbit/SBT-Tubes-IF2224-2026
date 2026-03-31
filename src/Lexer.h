#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <fstream>
#include <vector>
#include "Token.h"

class Lexer {
private:
    std::ifstream file; // Membaca langsung dari file .txt
    char currentChar;
    int currentLine;
    int currentColumn;

    // Helper functions untuk simulasi pergerakan DFA
    void advance();           // Membaca 1 karakter berikutnya dari file
    void skipWhitespace();    // Mengabaikan spasi, tab, newline
    
    // Fungsi-fungsi state pembantu (opsional, tergantung desain DFA masing" kita)
    Token lexNumber();        // Menangani INTCON dan REALCON
    Token lexStringOrChar();  // Menangani CHARCON dan STRING
    Token lexIdentifier();    // Menangani IDENT dan Keywords
    Token lexComment();       // Menangani COMMENT

public:
    // Constructor menerima path file .txt
    Lexer(const std::string& filename);
    
    // Destructor untuk menutup file
    ~Lexer();

    // Fungsi utama: meminta token selanjutnya (transisi DFA)
    Token getNextToken();

    // Mengambil semua token dan menyimpannya ke dalam vector
    std::vector<Token> tokenizeAll();
};

#endif // LEXER_H