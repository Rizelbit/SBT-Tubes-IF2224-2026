#pragma once

#include "AST.hpp"
#include "SymbolTable.hpp"
#include <string>
#include <vector>

struct SemanticError {
    std::string message;
};

class SemanticAnalyzer {
public:
    SemanticAnalyzer();
    explicit SemanticAnalyzer(SymbolTable& symbols);

    bool analyze(ASTNode* root);
    const std::vector<SemanticError>& errors() const;
    SymbolTable& symbolTable();
    const SymbolTable& symbolTable() const;

private:
    struct ConstantInfo {
        SemanticType type;
        int ordinal = 0;
        bool hasOrdinal = false;
        bool valid = false;
    };

    struct TypeInfo {
        SemanticType type;
        int low = 0;
        int high = 0;
        bool hasBounds = false;
    };

    SymbolTable ownedSymbols;
    SymbolTable* symbols;
    bool predefinedInitialized = false;
    std::vector<SemanticError> semanticErrors;

    void ensurePredefinedSymbols();
    void annotate(ASTNode* node);
    void visit(ASTNode* node);
    void visitProgram(ASTNode* node);
    void visitDeclarationPart(ASTNode* node);
    void visitBlock(ASTNode* node);
    void visitConstDecl(ASTNode* node);
    void visitTypeDecl(ASTNode* node);
    void visitVarDecl(ASTNode* node);
    void visitProcDecl(ASTNode* node);
    void visitFuncDecl(ASTNode* node);
    void visitParam(ASTNode* node);

    TypeInfo resolveType(ASTNode* node);
    TypeInfo resolveTypeReference(ASTNode* node);
    TypeInfo resolveRangeType(ASTNode* node);
    TypeInfo resolveArrayType(ASTNode* node);
    TypeInfo resolveRecordType(ASTNode* node);
    TypeInfo resolveEnumType(ASTNode* node);
    ConstantInfo evaluateConstant(ASTNode* node);

    bool isRecordFieldContext(ASTNode* node) const;
    bool containsReturnAssignment(ASTNode* node, const std::string& functionName) const;
    bool isOrdinal(const SemanticType& type) const;
    std::string normalize(const std::string& value) const;
    void reportError(const std::string& message);
};
