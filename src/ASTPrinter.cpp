#include "ASTPrinter.hpp"

std::string ASTPrinter::kindToString(ASTKind kind) {
    switch (kind) {
        case ASTKind::Program: return "Program";
        case ASTKind::Block: return "Block";
        case ASTKind::Assign: return "Assign";
        case ASTKind::Var: return "Var";
        case ASTKind::Literal: return "Literal";
        case ASTKind::Empty: return "EmptyStatement";
        default: return "Node";
    }
}

std::string ASTPrinter::typeKindToString(TypeKind kind) {
    switch (kind) {
        case TypeKind::Integer: return "Integer";
        case TypeKind::Real: return "Real";
        case TypeKind::String: return "String";
        default: return "Unknown";
    }
}

void ASTPrinter::print(ASTNode* root, std::ostream& out, const std::string& prefix, bool isLast) {
    if (!root) return;
    
    out << prefix << (prefix.empty() ? "" : (isLast ? "└── " : "├── "));
    out << kindToString(root->kind);
    if (!root->value.empty()) out << "(" << root->value << ")";
    
    out << " [type=" << typeKindToString(root->inferredType.kind) 
        << ", tab=" << root->tabIndex 
        << ", level=" << root->lexicalLevel << "]\n";

    std::string childPrefix = prefix + (prefix.empty() ? "" : (isLast ? "    " : "│   "));
    for (size_t i = 0; i < root->children.size(); ++i) {
        print(root->children[i], out, childPrefix, (i == root->children.size() - 1));
    }
}