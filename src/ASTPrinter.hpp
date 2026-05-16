#pragma once

#include "AST.hpp"
#include <iostream>
#include <string>

class ASTPrinter {
public:
    void print(ASTNode* root, std::ostream& out, const std::string& prefix = "", bool isLast = true);
private:
    std::string kindToString(ASTKind kind);
    std::string typeKindToString(TypeKind kind);
};