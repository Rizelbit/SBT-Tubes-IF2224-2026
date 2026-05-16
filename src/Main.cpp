#include "Lexer.hpp"
#include "Parser.hpp"
#include "ASTBuilder.hpp"
#include "ASTPrinter.hpp"
#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::string baseDir = "test/milestone-3/";

    std::string inputFile  = (argc > 1) ? (baseDir + argv[1]) : (baseDir + "input1.txt");
    std::string outputFile = (argc > 2) ? (baseDir + argv[2]) : (baseDir + "output1.txt");

    std::ofstream out(outputFile);
    if (!out.is_open()) {
        std::cerr << "Error: Cannot open output file '" << outputFile << "'\n";
        return 1;
    }

    try {
        Lexer lexer(inputFile);
        Parser parser(lexer);
        ParseNode* parseRoot = parser.parse();

        if (parser.isSuccess()) {
            std::cout << "Syntax Analysis: SUCCESS\n";
            
            ASTBuilder builder;
            ASTNode* astRoot = builder.build(parseRoot);

            // TODO: SemanticAnalyzer

            out << "Semantic Analysis: SUCCESS (Pending Semantic Checking)\n\n";
            out << "Decorated AST:\n";
            ASTPrinter printer;
            printer.print(astRoot, out);

            std::cout << "Pipeline complete. Check " << outputFile << "\n";
            delete astRoot;
        } else {
            std::cerr << "Semantic Analysis: FAILED (Syntax Error)\n";
            out << "Semantic Analysis: FAILED (Syntax Error)\n";
        }

        delete parseRoot;
    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}