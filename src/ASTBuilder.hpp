#pragma once

#include "ParseNode.hpp"
#include "AST.hpp"
#include <string>
#include <vector>

class ASTBuilder {
public:
    ASTNode* build(ParseNode* parseRoot);

private:
    ASTNode* buildProgram(ParseNode* node);
    ASTNode* buildDeclarationPart(ParseNode* node);
    ASTNode* buildConstDeclaration(ParseNode* node);
    ASTNode* buildTypeDeclaration(ParseNode* node);
    ASTNode* buildVarDeclaration(ParseNode* node);
    ASTNode* buildSubprogramDeclaration(ParseNode* node);
    ASTNode* buildParameterList(ParseNode* node);
    ASTNode* buildType(ParseNode* node);
    ASTNode* buildRecordFieldPart(ParseNode* node);
    ASTNode* buildBlock(ParseNode* node);
    ASTNode* buildStatement(ParseNode* node);
    ASTNode* buildExpression(ParseNode* node);
    ASTNode* buildVariable(ParseNode* node);
    ASTNode* buildProcedureCall(ParseNode* node);
    ASTNode* buildConstant(ParseNode* node);

    ASTNode* clone(ASTNode* node);
    std::vector<std::string> collectIdentifiers(ParseNode* node);
    std::string extractTokenKind(const std::string& s) const;
    std::string extractTokenValue(const std::string& s) const;
    bool isToken(ParseNode* node, const std::string& tokenKind) const;
    bool isOperatorWrapper(ParseNode* node) const;
    std::string operatorText(ParseNode* node) const;
};
