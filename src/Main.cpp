#include "Lexer.hpp"
#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::string baseDir = "test/milestone-1/";

    std::string inputFile  = (argc > 1) ? (baseDir + argv[1]) : (baseDir + "test1.txt");
    std::string outputFile = (argc > 2) ? (baseDir + argv[2]) : (baseDir + "output.txt");

    std::ofstream out(outputFile);
    if (!out.is_open()) {
        std::cerr << "Error: Cannot open output file '" << outputFile << "'\n";
        return 1;
    }

    try {
        Lexer lexer(inputFile);

        while (true) {
            Token tok = lexer.getNextToken();
            
            if (tok.type == TokenType::UNKNOWN) {
                std::string errMsg = "Lexical Error at Line " + std::to_string(tok.line) + 
                                     ", Col " + std::to_string(tok.column) + ": " + tok.value;
                out << errMsg << "\n";
                std::cerr << errMsg << "\n";
            } 
            else {
                out << tok.toString() << "\n";
            }

            if (tok.type == TokenType::END_OF_FILE) break;
        }

    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << "\n";
        out << e.what() << "\n";
        return 1;
    }

    std::cout << "Tokenization complete. Output written to '" << outputFile << "'.\n";
    return 0;
}