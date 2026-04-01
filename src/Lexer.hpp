#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <fstream>
#include <vector>
#include "Token.hpp"

class Lexer {
private:
    std::ifstream file;     // Stream untuk membaca file .txt source code
    char currentChar;       // Karakter yang sedang dibaca saat ini
    int currentLine;        // Pelacakan baris untuk pesan error
    int currentColumn;      // Pelacakan kolom untuk pesan error

    // ====================================================================
    // FUNGSI UTILITAS DASAR (Dikerjakan Anggota 1 di Lexer.cpp)
    // ====================================================================
    void advance();         // Membaca 1 karakter berikutnya dari file, update line & column
    char peek();            // Mengintip 1 karakter berikutnya tanpa memajukan 'currentChar'
    void skipWhitespace();  // Mengabaikan spasi (' '), tab ('\t'), dan newline ('\n', '\r')

    // ====================================================================
    // FUNGSI PEMBANTU DFA
    // ====================================================================
    
    // 1. file: Lexer_Literals.cpp
    // Menangani nilai literal: intcon, realcon, charcon, string
    Token lexLiteral(); 

    // 2. file: Lexer_Operators.cpp
    // Menangani operator simbol: plus, minus, times, rdiv, eql, neq, gtr, geq, lss, leq, becomes
    Token lexOperator(); 

    // 3. file: Lexer_Keywords.cpp
    // Menangani kata: ident, semua keyword (constsy, varsy, dll), serta text-operator (notsy, andsy, orsy, idiv, imod)
    Token lexIdentifierOrKeyword(); 

    // 4. file: Lexer_DelimsComments.cpp
    // Menangani delimiter dan komentar: lparent, rparent, lbrack, rbrack, comma, semicolon, period, colon, comment
    Token lexDelimiterOrComment(); 

public:
    // Constructor: Membuka file .txt dan menginisialisasi state pembacaan
    Lexer(const std::string& filename);
    
    // Destructor: Menutup stream file
    ~Lexer();

    // FUNGSI UTAMA
    // Bertugas mengecek 'currentChar' dan melempar prosesnya ke salah satu dari 4 fungsi pembantu di atas
    Token getNextToken();

    // Fungsi tambahan untuk mengumpulkan seluruh token
    std::vector<Token> tokenizeAll();
};

#endif // LEXER_H