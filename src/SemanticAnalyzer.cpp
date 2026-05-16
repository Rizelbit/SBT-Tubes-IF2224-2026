#include "SemanticAnalyzer.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <set>

bool SemanticAnalyzer::analyze(ASTNode* root) {
    semanticErrors.clear();
    scopes.clear();
    scopes.push_back({});
    initializePredefinedSymbols();
    visit(root);
    return semanticErrors.empty();
}

const std::vector<SemanticError>& SemanticAnalyzer::errors() const {
    return semanticErrors;
}

void SemanticAnalyzer::initializePredefinedSymbols() {
    const std::vector<std::pair<std::string, TypeKind>> builtins = {
        {"integer", TypeKind::Integer},
        {"real", TypeKind::Real},
        {"char", TypeKind::Char},
        {"boolean", TypeKind::Boolean},
        {"string", TypeKind::String}
    };

    for (const auto& builtin : builtins) {
        TypeInfo info;
        info.type.kind = builtin.second;
        SymbolInfo symbol;
        symbol.objectType = builtin.second;
        symbol.declaredType = info;
        symbol.isType = true;
        scopes.back()[builtin.first] = symbol;
    }

    SymbolInfo trueSymbol;
    trueSymbol.objectType = TypeKind::Boolean;
    trueSymbol.declaredType.type.kind = TypeKind::Boolean;
    trueSymbol.isConstant = true;
    scopes.back()["true"] = trueSymbol;

    SymbolInfo falseSymbol = trueSymbol;
    scopes.back()["false"] = falseSymbol;
}

void SemanticAnalyzer::visit(ASTNode* node) {
    if (!node) {
        return;
    }

    switch (node->kind) {
        case ASTKind::Program:
            visitProgram(node);
            break;
        case ASTKind::DeclarationPart:
            visitDeclarationPart(node);
            break;
        case ASTKind::Block:
            visitBlock(node);
            break;
        case ASTKind::ConstDecl:
            visitConstDecl(node);
            break;
        case ASTKind::TypeDecl:
            visitTypeDecl(node);
            break;
        case ASTKind::VarDecl:
        case ASTKind::FieldDecl:
            visitVarDecl(node);
            break;
        case ASTKind::ProcDecl:
            visitProcDecl(node);
            break;
        case ASTKind::FuncDecl:
            visitFuncDecl(node);
            break;
        case ASTKind::Param:
            visitParam(node);
            break;
        default:
            break;
    }
}

void SemanticAnalyzer::visitProgram(ASTNode* node) {
    SymbolInfo programSymbol;
    programSymbol.objectType = TypeKind::Void;
    declareSymbol(node->value, programSymbol);

    node->inferredType.kind = TypeKind::Void;
    node->lexicalLevel = static_cast<int>(scopes.size()) - 1;

    for (ASTNode* child : node->children) {
        visit(child);
    }
}

void SemanticAnalyzer::visitDeclarationPart(ASTNode* node) {
    for (ASTNode* child : node->children) {
        visit(child);
    }
}

void SemanticAnalyzer::visitBlock(ASTNode* node) {
    for (ASTNode* child : node->children) {
        if (child && child->kind == ASTKind::DeclarationPart) {
            visit(child);
        }
    }
}

void SemanticAnalyzer::visitConstDecl(ASTNode* node) {
    ConstantInfo constant = node->children.empty() ? ConstantInfo{} : evaluateConstant(node->children.front());
    node->inferredType.kind = constant.valid ? constant.type : TypeKind::Unknown;

    SymbolInfo symbol;
    symbol.objectType = node->inferredType.kind;
    symbol.declaredType.type.kind = node->inferredType.kind;
    symbol.declaredType.low = constant.ordinal;
    symbol.declaredType.high = constant.ordinal;
    symbol.declaredType.hasBounds = constant.hasOrdinal;
    symbol.isConstant = true;

    if (!declareSymbol(node->value, symbol)) {
        reportError("Redeclaration of identifier '" + node->value + "'");
    }
}

void SemanticAnalyzer::visitTypeDecl(ASTNode* node) {
    TypeInfo type = node->children.empty() ? TypeInfo{} : resolveType(node->children.front());
    node->inferredType = type.type;

    SymbolInfo symbol;
    symbol.objectType = type.type.kind;
    symbol.declaredType = type;
    symbol.isType = true;

    if (!declareSymbol(node->value, symbol)) {
        reportError("Redeclaration of identifier '" + node->value + "'");
        return;
    }

    if (!node->children.empty() && node->children.front()->kind == ASTKind::EnumType) {
        int ordinal = 0;
        std::set<std::string> seen;
        for (ASTNode* enumerator : node->children.front()->children) {
            std::string name = enumerator ? enumerator->value : "";
            std::string key = normalize(name);
            if (seen.count(key)) {
                reportError("Duplicate enumerated identifier '" + name + "'");
                continue;
            }
            seen.insert(key);

            SymbolInfo enumConstant;
            enumConstant.objectType = TypeKind::Enumerated;
            enumConstant.declaredType = type;
            enumConstant.declaredType.low = ordinal++;
            enumConstant.isConstant = true;

            if (!declareSymbol(name, enumConstant)) {
                reportError("Redeclaration of identifier '" + name + "'");
            }
        }
    }
}

void SemanticAnalyzer::visitVarDecl(ASTNode* node) {
    TypeInfo type = node->children.empty() ? TypeInfo{} : resolveType(node->children.front());
    node->inferredType = type.type;

    SymbolInfo symbol;
    symbol.objectType = type.type.kind;
    symbol.declaredType = type;

    if (!declareSymbol(node->value, symbol)) {
        reportError("Redeclaration of identifier '" + node->value + "'");
    }
}

void SemanticAnalyzer::visitProcDecl(ASTNode* node) {
    SymbolInfo symbol;
    symbol.objectType = TypeKind::Procedure;

    if (!declareSymbol(node->value, symbol)) {
        reportError("Redeclaration of identifier '" + node->value + "'");
        return;
    }

    node->inferredType.kind = TypeKind::Procedure;
    scopes.push_back({});

    for (ASTNode* child : node->children) {
        if (!child) {
            continue;
        }

        if (child->kind == ASTKind::DeclarationPart && child->value == "parameters") {
            visitDeclarationPart(child);
        } else if (child->kind == ASTKind::Block) {
            visitBlock(child);
        }
    }

    scopes.pop_back();
}

void SemanticAnalyzer::visitFuncDecl(ASTNode* node) {
    ASTNode* returnTypeNode = nullptr;
    ASTNode* parameterPart = nullptr;
    ASTNode* body = nullptr;

    for (ASTNode* child : node->children) {
        if (!child) {
            continue;
        }

        if (child->kind == ASTKind::Type) {
            returnTypeNode = child;
        } else if (child->kind == ASTKind::DeclarationPart && child->value == "parameters") {
            parameterPart = child;
        } else if (child->kind == ASTKind::Block) {
            body = child;
        }
    }

    TypeInfo returnType = returnTypeNode ? resolveType(returnTypeNode) : TypeInfo{};
    if (!returnTypeNode || returnType.type.kind == TypeKind::Unknown) {
        reportError("Invalid function return type for '" + node->value + "'");
    }

    SymbolInfo symbol;
    symbol.objectType = TypeKind::Function;
    symbol.declaredType = returnType;

    if (!declareSymbol(node->value, symbol)) {
        reportError("Redeclaration of identifier '" + node->value + "'");
        return;
    }

    node->inferredType = returnType.type;
    scopes.push_back({});

    if (parameterPart) {
        visitDeclarationPart(parameterPart);
    }

    if (body) {
        visitBlock(body);
    }

    if (!body || !containsReturnAssignment(body, node->value)) {
        reportError("Function '" + node->value + "' does not assign a return value");
    }

    scopes.pop_back();
}

void SemanticAnalyzer::visitParam(ASTNode* node) {
    TypeInfo type = node->children.empty() ? TypeInfo{} : resolveType(node->children.front());
    node->inferredType = type.type;

    SymbolInfo symbol;
    symbol.objectType = type.type.kind;
    symbol.declaredType = type;

    if (!declareSymbol(node->value, symbol)) {
        reportError("Redeclaration of identifier '" + node->value + "'");
    }
}

SemanticAnalyzer::TypeInfo SemanticAnalyzer::resolveType(ASTNode* node) {
    if (!node) {
        return TypeInfo{};
    }

    switch (node->kind) {
        case ASTKind::Type:
            return resolveTypeReference(node);
        case ASTKind::RangeType:
            return resolveRangeType(node);
        case ASTKind::ArrayType:
            return resolveArrayType(node);
        case ASTKind::RecordType:
            return resolveRecordType(node);
        case ASTKind::EnumType:
            return resolveEnumType(node);
        default:
            return TypeInfo{};
    }
}

SemanticAnalyzer::TypeInfo SemanticAnalyzer::resolveTypeReference(ASTNode* node) {
    TypeInfo type;
    const SymbolInfo* symbol = lookupSymbol(node->value);

    if (!symbol || !symbol->isType) {
        reportError("Unknown type '" + node->value + "'");
        type.type.kind = TypeKind::Unknown;
        node->inferredType = type.type;
        return type;
    }

    type = symbol->declaredType;
    node->inferredType = type.type;
    return type;
}

SemanticAnalyzer::TypeInfo SemanticAnalyzer::resolveRangeType(ASTNode* node) {
    TypeInfo type;

    if (node->children.size() < 2) {
        reportError("Invalid subrange type");
        return type;
    }

    ConstantInfo low = evaluateConstant(node->children[0]);
    ConstantInfo high = evaluateConstant(node->children[1]);

    if (!low.valid || !high.valid) {
        reportError("Invalid constant in subrange type");
        return type;
    }

    if (low.type == TypeKind::Real || high.type == TypeKind::Real) {
        reportError("Subrange cannot use Real type");
        return type;
    }

    if (low.type != high.type) {
        reportError("Subrange bounds must have the same type");
        return type;
    }

    if (low.hasOrdinal && high.hasOrdinal && low.ordinal > high.ordinal) {
        reportError("Invalid range: lower bound greater than upper bound");
        return type;
    }

    type.type.kind = TypeKind::Subrange;
    type.low = low.ordinal;
    type.high = high.ordinal;
    type.hasBounds = low.hasOrdinal && high.hasOrdinal;
    node->inferredType = type.type;
    return type;
}

SemanticAnalyzer::TypeInfo SemanticAnalyzer::resolveArrayType(ASTNode* node) {
    TypeInfo type;
    type.type.kind = TypeKind::Array;

    if (node->children.size() < 2) {
        reportError("Invalid array type");
        return type;
    }

    TypeInfo indexType = resolveType(node->children[0]);
    TypeInfo elementType = resolveType(node->children[1]);

    if (indexType.type.kind == TypeKind::Real) {
        reportError("Array index type cannot be Real");
    }

    if (!isOrdinal(indexType.type.kind) && indexType.type.kind != TypeKind::Subrange) {
        reportError("Array index type must be ordinal");
    }

    node->inferredType = type.type;
    if (elementType.type.kind == TypeKind::Unknown) {
        type.type.kind = TypeKind::Unknown;
    }
    return type;
}

SemanticAnalyzer::TypeInfo SemanticAnalyzer::resolveRecordType(ASTNode* node) {
    TypeInfo type;
    type.type.kind = TypeKind::Record;
    std::set<std::string> fields;

    for (ASTNode* field : node->children) {
        if (!field || field->kind != ASTKind::FieldDecl) {
            continue;
        }

        std::string key = normalize(field->value);
        if (fields.count(key)) {
            reportError("Duplicate field '" + field->value + "' in record");
            continue;
        }
        fields.insert(key);

        if (!field->children.empty()) {
            TypeInfo fieldType = resolveType(field->children.front());
            field->inferredType = fieldType.type;
        }
    }

    node->inferredType = type.type;
    return type;
}

SemanticAnalyzer::TypeInfo SemanticAnalyzer::resolveEnumType(ASTNode* node) {
    TypeInfo type;
    type.type.kind = TypeKind::Enumerated;
    type.low = 0;
    type.high = static_cast<int>(node->children.size()) - 1;
    type.hasBounds = true;
    node->inferredType = type.type;
    return type;
}

SemanticAnalyzer::ConstantInfo SemanticAnalyzer::evaluateConstant(ASTNode* node) {
    ConstantInfo constant;

    if (!node) {
        return constant;
    }

    if (node->kind == ASTKind::Literal) {
        constant.valid = true;
        bool numeric = !node->value.empty();
        bool real = false;

        for (char c : node->value) {
            if (c == '.') {
                real = true;
            } else if (!std::isdigit(static_cast<unsigned char>(c)) && c != '-') {
                numeric = false;
                break;
            }
        }

        if (numeric && real) {
            constant.type = TypeKind::Real;
        } else if (numeric) {
            constant.type = TypeKind::Integer;
            constant.ordinal = std::atoi(node->value.c_str());
            constant.hasOrdinal = true;
        } else if (node->value.size() == 1) {
            constant.type = TypeKind::Char;
            constant.ordinal = static_cast<unsigned char>(node->value[0]);
            constant.hasOrdinal = true;
        } else {
            constant.type = TypeKind::String;
        }

        node->inferredType.kind = constant.type;
        return constant;
    }

    if (node->kind == ASTKind::Var) {
        const SymbolInfo* symbol = lookupSymbol(node->value);
        if (!symbol || !symbol->isConstant) {
            reportError("Undeclared constant '" + node->value + "'");
            return constant;
        }

        constant.type = symbol->objectType;
        constant.ordinal = symbol->declaredType.low;
        constant.hasOrdinal = symbol->declaredType.hasBounds;
        constant.valid = true;
        node->inferredType.kind = constant.type;
        return constant;
    }

    return constant;
}

bool SemanticAnalyzer::declareSymbol(const std::string& name, const SymbolInfo& symbol) {
    std::string key = normalize(name);
    if (key.empty()) {
        return true;
    }

    if (scopes.empty()) {
        scopes.push_back({});
    }

    auto& current = scopes.back();
    if (current.find(key) != current.end()) {
        return false;
    }

    current[key] = symbol;
    return true;
}

const SemanticAnalyzer::SymbolInfo* SemanticAnalyzer::lookupSymbol(const std::string& name) const {
    std::string key = normalize(name);
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto found = it->find(key);
        if (found != it->end()) {
            return &found->second;
        }
    }
    return nullptr;
}

bool SemanticAnalyzer::containsReturnAssignment(ASTNode* node, const std::string& functionName) const {
    if (!node) {
        return false;
    }

    if (node->kind == ASTKind::Assign && !node->children.empty()) {
        ASTNode* target = node->children.front();
        if (target && target->kind == ASTKind::Var && normalize(target->value) == normalize(functionName)) {
            return true;
        }
    }

    for (ASTNode* child : node->children) {
        if (containsReturnAssignment(child, functionName)) {
            return true;
        }
    }

    return false;
}

bool SemanticAnalyzer::isOrdinal(TypeKind kind) const {
    return kind == TypeKind::Integer ||
           kind == TypeKind::Char ||
           kind == TypeKind::Boolean ||
           kind == TypeKind::Enumerated;
}

std::string SemanticAnalyzer::normalize(const std::string& value) const {
    std::string result = value;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return result;
}

std::string SemanticAnalyzer::typeName(TypeKind kind) const {
    switch (kind) {
        case TypeKind::Unknown: return "Unknown";
        case TypeKind::Void: return "Void";
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
    }
    return "Unknown";
}

void SemanticAnalyzer::reportError(const std::string& message) {
    semanticErrors.push_back(SemanticError{message});
}
