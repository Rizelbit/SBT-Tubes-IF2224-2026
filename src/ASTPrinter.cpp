#include "ASTPrinter.hpp"

std::string ASTPrinter::kindToString(ASTKind kind) {
    switch (kind) {
        case ASTKind::Program: return "Program";
        case ASTKind::DeclarationPart: return "Declarations";
        case ASTKind::Block: return "Block";
        case ASTKind::ConstDecl: return "ConstDecl";
        case ASTKind::TypeDecl: return "TypeDecl";
        case ASTKind::VarDecl: return "VarDecl";
        case ASTKind::FieldDecl: return "FieldDecl";
        case ASTKind::ProcDecl: return "ProcedureDecl";
        case ASTKind::FuncDecl: return "FunctionDecl";
        case ASTKind::Param: return "Param";
        case ASTKind::Type: return "Type";
        case ASTKind::ArrayType: return "ArrayType";
        case ASTKind::RecordType: return "RecordType";
        case ASTKind::RangeType: return "RangeType";
        case ASTKind::EnumType: return "EnumType";
        case ASTKind::Assign: return "Assign";
        case ASTKind::If: return "If";
        case ASTKind::While: return "While";
        case ASTKind::Repeat: return "Repeat";
        case ASTKind::For: return "For";
        case ASTKind::Case: return "Case";
        case ASTKind::ProcedureCall: return "ProcedureCall";
        case ASTKind::FunctionCall: return "FunctionCall";
        case ASTKind::BinOp: return "BinOp";
        case ASTKind::UnaryOp: return "UnaryOp";
        case ASTKind::Literal: return "Literal";
        case ASTKind::Var: return "Var";
        case ASTKind::ArrayAccess: return "ArrayAccess";
        case ASTKind::FieldAccess: return "FieldAccess";
        case ASTKind::Empty: return "EmptyStatement";
        default: return "Node";
    }
}

std::string ASTPrinter::typeKindToString(TypeKind kind) {
    switch (kind) {
        case TypeKind::Integer: return "Integer";
        case TypeKind::Real: return "Real";
        case TypeKind::Char: return "Char";
        case TypeKind::Boolean: return "Boolean";
        case TypeKind::String: return "String";
        case TypeKind::Subrange: return "Subrange";
        case TypeKind::Enumerated: return "Enumerated";
        case TypeKind::Array: return "Array";
        case TypeKind::Record: return "Record";
        case TypeKind::Procedure: return "Procedure";
        case TypeKind::Function: return "Function";
        case TypeKind::Void: return "Void";
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
        << ", block=" << root->blockIndex
        << ", level=" << root->lexicalLevel << "]\n";

    std::string childPrefix = prefix;
    if (!isRoot) {
        childPrefix += (isLast ? "    " : "│   ");
    }

    for (size_t i = 0; i < root->children.size(); ++i) {
        print(root->children[i], out, childPrefix, (i == root->children.size() - 1), false);
    }
}
