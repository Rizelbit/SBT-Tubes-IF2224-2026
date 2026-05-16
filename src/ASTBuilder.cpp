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
    for (auto child : node->children) {\
        if (child->name == "<declaration-part>") { // ini sementara cuma buat testing AST builder, tolong disesuaikan orang 3
            ast->addChild(new ASTNode(ASTKind::VarDecl, "Variables Declaration Block"));
            // TODO: <declaration-part> handler
        } else if (child->name == "<compound-statement>") {
            ast->addChild(buildBlock(child));
        }
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
    } else if (firstChild->name == "<procedure/function-call>") {
        ASTNode* callNode = new ASTNode(ASTKind::ProcedureCall, extractTokenValue(firstChild->children[0]->name));
        if (firstChild->children.size() > 2 && firstChild->children[2]->name == "<parameter-list>") {
            ParseNode* paramList = firstChild->children[2];
            for (auto p : paramList->children) {
                if (p->name == "<expression>") {
                    callNode->addChild(buildExpression(p));
                }
            }
        }
        return callNode;
    }

    return new ASTNode(ASTKind::Empty);
}

ASTNode* ASTBuilder::buildExpression(ParseNode* node) {
    if (!node) return nullptr;
    
    if (node->name.find("IDENT") != std::string::npos) {
        return new ASTNode(ASTKind::Var, extractTokenValue(node->name));
    }
    if (node->name.find("INTCON") != std::string::npos || node->name.find("STRING") != std::string::npos) {
        return new ASTNode(ASTKind::Literal, extractTokenValue(node->name));
    }

    if (node->children.size() == 3) {
        std::string midName = node->children[1]->name;
        if (midName == "<additive-operator>" || midName == "<multiplicative-operator>") {
            std::string opSymbol = extractTokenValue(node->children[1]->children[0]->name);
            if (opSymbol.empty()) opSymbol = node->children[1]->children[0]->name;
            
            ASTNode* binOp = new ASTNode(ASTKind::BinOp, opSymbol);
            binOp->addChild(buildExpression(node->children[0]));
            binOp->addChild(buildExpression(node->children[2]));
            return binOp;
        }
    }

    for (auto child : node->children) {
        ASTNode* result = buildExpression(child);
        if (result && result->kind != ASTKind::Empty) {
            return result;
        }
    }
    
    return new ASTNode(ASTKind::Empty);
}