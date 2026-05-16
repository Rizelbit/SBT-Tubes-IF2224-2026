#pragma once

#include "AST.hpp"
#include <string>
#include <unordered_map>
#include <vector>

struct SemanticError {
    std::string message;
};

class SemanticAnalyzer {
public:
    bool analyze(ASTNode* root);
    const std::vector<SemanticError>& errors() const;

private:
    struct ConstantInfo {
        TypeKind type = TypeKind::Unknown;
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

    struct SymbolInfo {
        TypeKind objectType = TypeKind::Unknown;
        TypeInfo declaredType;
        bool isType = false;
        bool isConstant = false;
    };

    std::vector<SemanticError> semanticErrors;
    std::vector<std::unordered_map<std::string, SymbolInfo>> scopes;

    void initializePredefinedSymbols();
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

    bool declareSymbol(const std::string& name, const SymbolInfo& symbol);
    const SymbolInfo* lookupSymbol(const std::string& name) const;
    bool containsReturnAssignment(ASTNode* node, const std::string& functionName) const;
    bool isOrdinal(TypeKind kind) const;
    std::string normalize(const std::string& value) const;
    std::string typeName(TypeKind kind) const;
    void reportError(const std::string& message);
};
