#include "Lexer.hpp"
#include "Parser.hpp"
#include "ASTBuilder.hpp"
#include "ASTPrinter.hpp"
#include "SemanticAnalyzer.hpp"
#include "DecoratedASTReader.hpp"
#include "CodeGenerator.hpp"
#include "IntermediateCode.hpp"
#include "Interpreter.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

namespace {

bool generateAndRun(ASTNode* astRoot, const SymbolTable& symbols, std::ofstream& out) {
    CodeGenerator cg(symbols);
    CodeBuffer buf = cg.generate(astRoot);

    out << "\nIntermediate Code:\n";
    buf.print(out);
    out << "\n";

    std::cout << "Intermediate Code: GENERATED (" << buf.instructions().size() << " instructions)\n";

    std::unordered_map<int, int> addrToTabIndex;
    for (const auto& kv : cg.subprogramAddr()) {
        addrToTabIndex[kv.second] = kv.first;
    }

    Interpreter interpreter(buf, &symbols, &addrToTabIndex);
    bool runtimeSuccess = interpreter.run();

    out << "Program Output:\n";
    out << interpreter.output();
    if (!interpreter.output().empty() && interpreter.output().back() != '\n') {
        out << "\n";
    }
    out << "\n";

    if (runtimeSuccess && !interpreter.hasErrors()) {
        out << "Runtime Status: SUCCESS\n";
        return true;
    }

    out << "Runtime Status: FAILED\n";
    out << "Runtime Error:\n";
    for (const auto& err : interpreter.errors()) {
        out << "- " << err.message << " at instruction " << err.instructionAddress << "\n";
    }
    return false;
}

} // namespace

int main(int argc, char* argv[]) {
    const std::string baseDir = "test/milestone-4/";

    std::string inputFile  = (argc > 1) ? argv[1] : (baseDir + "input1.txt");
    std::string outputFile = (argc > 2) ? argv[2] : (baseDir + "output1.txt");

    if (argc > 1 && inputFile.find('/') == std::string::npos && inputFile.find('\\') == std::string::npos) {
        inputFile = baseDir + inputFile;
    }
    if (argc > 2 && outputFile.find('/') == std::string::npos && outputFile.find('\\') == std::string::npos) {
        outputFile = baseDir + outputFile;
    }

    std::ofstream out(outputFile);
    if (!out.is_open()) {
        std::cerr << "Error: Cannot open output file '" << outputFile << "'\n";
        return 1;
    }

    try {
        DecoratedASTReader decoratedReader;
        if (decoratedReader.canRead(inputFile)) {
            DecoratedASTInput decorated = decoratedReader.read(inputFile);

            std::cout << "Input Mode: DECORATED AST\n";
            out << "Input Mode: DECORATED AST\n";
            out << "Semantic Analysis: SKIPPED (Decorated AST input)\n\n";

            generateAndRun(decorated.root, decorated.symbolTable, out);
            std::cout << "Pipeline complete. Check " << outputFile << "\n";
            return 0;
        }

        Lexer lexer(inputFile);
        Parser parser(lexer);
        ParseNode* parseRoot = parser.parse();

        if (parser.isSuccess()) {
            std::cout << "Syntax Analysis: SUCCESS\n";
            out << "Syntax Analysis: SUCCESS\n";
            
            ASTBuilder builder;
            ASTNode* astRoot = builder.build(parseRoot);

            SemanticAnalyzer analyzer;
            bool semanticSuccess = analyzer.analyze(astRoot);

            out << "Semantic Analysis: " << (semanticSuccess ? "SUCCESS" : "FAILED") << "\n\n";
            if (!semanticSuccess) {
                out << "Semantic Errors:\n";
                for (const SemanticError& error : analyzer.errors()) {
                    out << "- " << error.message << "\n";
                }
                out << "\n";
                out << "Intermediate Code: NOT GENERATED\n";
                out << "Runtime Status: NOT EXECUTED\n\n";
            }

            out << "Decorated AST:\n";
            ASTPrinter printer;
            printer.print(astRoot, out);
            out << "\n";
            analyzer.symbolTable().printTables(out);

            if (semanticSuccess) {
                generateAndRun(astRoot, analyzer.symbolTable(), out);
            } else {
                out << "Intermediate Code: NOT GENERATED\n";
                out << "Runtime Status: NOT EXECUTED\n";
            }

            std::cout << "Semantic Analysis: " << (semanticSuccess ? "SUCCESS" : "FAILED") << "\n";
            std::cout << "Pipeline complete. Check " << outputFile << "\n";
            delete astRoot;
        } else {
            std::cerr << "Semantic Analysis: FAILED (Syntax Error)\n";
            out << "Syntax Analysis: FAILED\n";
            out << "Semantic Analysis: NOT EXECUTED\n";
            out << "Intermediate Code: NOT GENERATED\n";
            out << "Runtime Status: NOT EXECUTED\n";
        }

        delete parseRoot;
    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
