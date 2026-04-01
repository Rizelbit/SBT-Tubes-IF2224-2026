#include "Lexer.hpp"
#include <fstream>
#include <iostream>

int main(int argc, char* argv[]) {
    std::string inputFile  = (argc > 1) ? argv[1] : "test/milestone-1/test1.txt";
    std::string outputFile = (argc > 2) ? argv[2] : "test/milestone-1/output.txt";

    std::ofstream out(outputFile);
    if (!out.is_open()) {
        std::cerr << "Error: Cannot open output file '" << outputFile << "'\n";
        return 1;
    }

    try {
        Lexer lexer(inputFile);

        while (true) {
            Token tok = lexer.getNextToken();
            out << tok.toString() << "\n";
            if (tok.type == TokenType::END_OF_FILE) break;
        }

    } catch (const std::runtime_error& e) {
        // Cetak error leksikal ke stdout DAN output file
        std::cerr << e.what() << "\n";
        out << e.what() << "\n";
        return 1;
    }

    std::cout << "Tokenization complete. Output written to '" << outputFile << "'.\n";
    return 0;
}