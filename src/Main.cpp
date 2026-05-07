#include "Lexer.hpp"
#include "Parser.hpp"
#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::string baseDir = "test/milestone-2/";

    std::string inputFile  = (argc > 1) ? (baseDir + argv[1]) : (baseDir + "test1.txt");
    std::string outputFile = (argc > 2) ? (baseDir + argv[2]) : (baseDir + "output.txt");

    std::ofstream out(outputFile);
    if (!out.is_open()) {
        std::cerr << "Error: Cannot open output file '" << outputFile << "'\n";
        return 1;
    }

    try {
        Lexer lexer(inputFile);
        Parser parser(lexer);
        ParseNode* parseTreeRoot = parser.parse();

        if (parser.isSuccess()) {
            parseTreeRoot->printTree(out);
            std::cout << "Parsing complete. Parse Tree written to '" << outputFile << "'.\n";
        } else {
            std::cerr << "Parsing failed due to Syntax Errors.\n";
            out << "Parsing failed due to Syntax Errors.\n";
        }

        delete parseTreeRoot;

    } catch (const std::runtime_error& e) {
        std::cerr << "Fatal Error: " << e.what() << "\n";
        out << "Fatal Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}