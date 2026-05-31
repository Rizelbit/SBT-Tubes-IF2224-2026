#pragma once
#include "AST.hpp"
#include "IntermediateCode.hpp"
#include "SymbolTable.hpp"

#include <string>
#include <unordered_map>
#include <vector>

class CodeGenerator {
public:
    explicit CodeGenerator(const SymbolTable& symbols);

    CodeBuffer generate(ASTNode* root);

    CodeBuffer& buffer() { return buf_; }
    const CodeBuffer& buffer() const { return buf_; }

protected:

    const SymbolTable& sym_;
    CodeBuffer buf_;

    void generateNode(ASTNode* node);

    // program dan block structure
    void generateProgram(ASTNode* node);
    void generateBlock(ASTNode* node);
    void generateDeclarationPart(ASTNode* node);
    void generateSubprogramDecl(ASTNode* node);

    // statement
    void generateStatement(ASTNode* node);
    void generateAssign(ASTNode* node);

    void generateExpression(ASTNode* node);
    void generateLiteral(ASTNode* node);
    void generateVar(ASTNode* node);
    void generateBinOp(ASTNode* node);
    void generateUnaryOp(ASTNode* node);
    void generateArrayAccess(ASTNode* node); // rvalue: LOD hasil indeks
    void generateFieldAccess(ASTNode* node); // rvalue: LOD hasil offset

    // alamat LHS buat assignment
    struct LValue { int level; int address; bool isStack = false; };
    LValue resolveLValue(ASTNode* node);
    LValue resolveLValueArrayAccess(ASTNode* node);
    LValue resolveLValueFieldAccess(ASTNode* node);

    virtual void generateIf(ASTNode* node);
    virtual void generateWhile(ASTNode* node);
    virtual void generateRepeat(ASTNode* node);
    virtual void generateFor(ASTNode* node);
    virtual void generateCase(ASTNode* node);

    virtual void generateProcedureCall(ASTNode* node);
    virtual void generateFunctionCall(ASTNode* node);

    void generateWrite(ASTNode* node, bool newline);
    void generateRead(ASTNode* node, bool newline);
    void generateCaseBranch(ASTNode* selector, ASTNode* branch, std::vector<int>& endJumps);
    void emitSubprogramCall(ASTNode* node, const TabEntry& entry);
    int typeSize(const SemanticType& t) const;

    int oprForBinOp(const std::string& op) const;
    int oprForUnaryOp(const std::string& op) const;

    static std::string normalize(const std::string& s);

    int currentLevel_ = 0;

    std::string currentFunctionName_;

    std::unordered_map<int, int> subprogramAddr_;
    std::unordered_map<int, std::vector<int>> pendingSubprogramCalls_;

    static constexpr int FRAME_HEADER = 3;
};
