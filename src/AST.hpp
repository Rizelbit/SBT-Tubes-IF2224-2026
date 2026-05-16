#pragma once

#include <string>
#include <vector>

enum class TypeKind {
    Unknown, Void, Integer, Real, Char, Boolean, String, Subrange, Enumerated,
    Array, Record, Procedure, Function
};

struct SemanticType {
    TypeKind kind = TypeKind::Unknown;
    int ref = 0;
};

enum class ASTKind {
    Program, DeclarationPart, Block, ConstDecl, TypeDecl, VarDecl, FieldDecl,
    ProcDecl, FuncDecl, Param, Type, ArrayType, RecordType, RangeType, EnumType,
    Assign, If, While, Repeat, For, Case, ProcedureCall, FunctionCall,
    BinOp, UnaryOp, Literal, Var, ArrayAccess, FieldAccess, Empty
};

struct ASTNode {
    ASTKind kind;
    std::string value;
    std::vector<ASTNode*> children;

    SemanticType inferredType;
    int tabIndex = -1;
    int blockIndex = -1;
    int lexicalLevel = 0;

    ASTNode(ASTKind kind, const std::string& value = "") : kind(kind), value(value) {}

    ~ASTNode() {
        for (auto child : children) delete child;
    }

    void addChild(ASTNode* child) {
        if (child) children.push_back(child);
    }
};
