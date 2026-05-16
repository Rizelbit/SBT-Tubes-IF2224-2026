#pragma once

#include "ParseNode.hpp"
#include "AST.hpp"
#include <string>

class ASTBuilder {
public:
    ASTNode* build(ParseNode* parseRoot);

private:
    ASTNode* buildProgram(ParseNode* node);
    ASTNode* buildBlock(ParseNode* node);
    ASTNode* buildStatement(ParseNode* node);
    ASTNode* buildExpression(ParseNode* node);

    std::string extractTokenKind(const std::string& s);
    std::string extractTokenValue(const std::string& s);
};