#include "ASTPrinter.hpp"

std::string ASTPrinter::kindToString(ASTKind kind) {
    switch (kind) {
        case ASTKind::Program: return "Program";
        case ASTKind::Block: return "Block";
        case ASTKind::Assign: return "Assign";
        case ASTKind::Var: return "Var";
        case ASTKind::Literal: return "Literal";
        case ASTKind::Empty: return "EmptyStatement";
        case ASTKind::VarDecl: return "VarDecl";
        case ASTKind::BinOp: return "BinOp";
        case ASTKind::ProcedureCall: return "ProcedureCall";
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

void ASTPrinter::print(ASTNode* root, std::ostream& out, const std::string& prefix, bool isLast, bool isRoot) {
    if (!root) return;
    
    if (!isRoot) {
        out << prefix << (isLast ? "└── " : "├── ");
    } else {
        out << prefix;
    }
    
    out << kindToString(root->kind);
    if (!root->value.empty()) out << "(" << root->value << ")";
    
    out << " [type=" << typeKindToString(root->inferredType.kind) 
        << ", tab=" << root->tabIndex 
        << ", level=" << root->lexicalLevel << "]\n";

    std::string childPrefix = prefix;
    if (!isRoot) {
        childPrefix += (isLast ? "    " : "│   ");
    }

    for (size_t i = 0; i < root->children.size(); ++i) {
        print(root->children[i], out, childPrefix, (i == root->children.size() - 1), false);
    }
}