#pragma once

#include "AST.hpp"
#include "SymbolTable.hpp"
#include <string>
#include <utility>

struct DecoratedASTInput {
    ASTNode* root = nullptr;
    SymbolTable symbolTable;

    ~DecoratedASTInput() {
        delete root;
    }

    DecoratedASTInput() = default;
    DecoratedASTInput(const DecoratedASTInput&) = delete;
    DecoratedASTInput& operator=(const DecoratedASTInput&) = delete;

    DecoratedASTInput(DecoratedASTInput&& other) noexcept
        : root(other.root), symbolTable(std::move(other.symbolTable)) {
        other.root = nullptr;
    }

    DecoratedASTInput& operator=(DecoratedASTInput&& other) noexcept {
        if (this != &other) {
            delete root;
            root = other.root;
            symbolTable = std::move(other.symbolTable);
            other.root = nullptr;
        }
        return *this;
    }
};

class DecoratedASTReader {
public:
    bool canRead(const std::string& filename) const;
    DecoratedASTInput read(const std::string& filename) const;
};
