#include "ASTBuilder.hpp"
#include <iostream>

std::string ASTBuilder::extractTokenKind(const std::string& s) {
    size_t pos = s.find('(');
    if (pos != std::string::npos) return s.substr(0, pos);
    return s;
}

std::string ASTBuilder::extractTokenValue(const std::string& s) {
    size_t pos1 = s.find('(');
    size_t pos2 = s.find(')');
    if (pos1 != std::string::npos && pos2 != std::string::npos) {
        return s.substr(pos1 + 1, pos2 - pos1 - 1);
    }
    return s;
}

ASTNode* ASTBuilder::build(ParseNode* parseRoot) {
    if (!parseRoot) return nullptr;
    if (parseRoot->name == "<program>") return buildProgram(parseRoot);
    return nullptr;
}

ASTNode* ASTBuilder::buildProgram(ParseNode* node) {
    ASTNode* ast = new ASTNode(ASTKind::Program);
    if (node->children.size() > 0) {
        ParseNode* header = node->children[0];
        if (header->children.size() > 1) {
            ast->value = extractTokenValue(header->children[1]->name);
        }
    }
    for (auto child : node->children) {
        if (child->name == "<compound-statement>") {
            ast->addChild(buildBlock(child));
        }
        // TODO: <declaration-part> handler
    }
    return ast;
}

ASTNode* ASTBuilder::buildBlock(ParseNode* node) {
    ASTNode* ast = new ASTNode(ASTKind::Block);
    for (auto child : node->children) {
        if (child->name == "<statement-list>") {
            for (auto stmtChild : child->children) {
                if (stmtChild->name == "<statement>") {
                    ASTNode* s = buildStatement(stmtChild);
                    if (s) ast->addChild(s);
                }
            }
        }
    }
    return ast;
}

ASTNode* ASTBuilder::buildStatement(ParseNode* node) {
    if (node->children.empty()) return new ASTNode(ASTKind::Empty);

    ParseNode* firstChild = node->children[0];
    if (firstChild->name == "<assignment-statement>") {
        ASTNode* assignNode = new ASTNode(ASTKind::Assign);
        assignNode->addChild(buildExpression(firstChild->children[0]));
        assignNode->addChild(buildExpression(firstChild->children[2]));

        return assignNode;
    }

    return new ASTNode(ASTKind::Empty);
}

ASTNode* ASTBuilder::buildExpression(ParseNode* node) {
    if (node->name.find("IDENT") != std::string::npos) {
        return new ASTNode(ASTKind::Var, extractTokenValue(node->name));
    } else if (node->name.find("INTCON") != std::string::npos) {
        return new ASTNode(ASTKind::Literal, extractTokenValue(node->name));
    }
    return new ASTNode(ASTKind::Empty);
}