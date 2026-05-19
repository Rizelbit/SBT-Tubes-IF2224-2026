#include "SemanticAnalyzer.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <set>

SemanticAnalyzer::SemanticAnalyzer() : ownedSymbols(), symbols(&ownedSymbols) {}

SemanticAnalyzer::SemanticAnalyzer(SymbolTable& symbols) : ownedSymbols(), symbols(&symbols) {}

bool SemanticAnalyzer::analyze(ASTNode* root) {
    semanticErrors.clear();
    ensurePredefinedSymbols();
    visit(root);
    return semanticErrors.empty();
}

const std::vector<SemanticError>& SemanticAnalyzer::errors() const {
    return semanticErrors;
}

SymbolTable& SemanticAnalyzer::symbolTable() {
    return *symbols;
}

const SymbolTable& SemanticAnalyzer::symbolTable() const {
    return *symbols;
}

void SemanticAnalyzer::ensurePredefinedSymbols() {
    if (!predefinedInitialized) {
        symbols->initPredefined();
        predefinedInitialized = true;
    }
}

void SemanticAnalyzer::annotate(ASTNode* node) {
    if (!node) {
        return;
    }
    node->lexicalLevel = symbols->currentLevel();
    node->blockIndex = symbols->currentBlock();
}

void SemanticAnalyzer::visit(ASTNode* node) {
    if (!node) {
        return;
    }

    annotate(node);

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
    SemanticType type = makeVoidType();
    node->inferredType = type;
    node->tabIndex = symbols->declare(node->value, ObjectKind::Program, type, 1, 0);

    if (node->tabIndex == -1) {
        reportError("Redeclaration of program identifier '" + node->value + "'");
    }

    annotate(node);

    for (ASTNode* child : node->children) {
        visit(child);
    }
}

void SemanticAnalyzer::visitDeclarationPart(ASTNode* node) {
    annotate(node);
    for (ASTNode* child : node->children) {
        visit(child);
    }
}

void SemanticAnalyzer::visitBlock(ASTNode* node) {
    annotate(node);
    for (ASTNode* child : node->children) {
        if (child && child->kind == ASTKind::DeclarationPart) {
            visit(child);
        }
    }
}

void SemanticAnalyzer::visitConstDecl(ASTNode* node) {
    annotate(node);
    ConstantInfo constant = node->children.empty() ? ConstantInfo{} : evaluateConstant(node->children.front());
    node->inferredType = constant.valid ? constant.type : makeUnknownType();

    int adr = constant.hasOrdinal ? constant.ordinal : 0;
    node->tabIndex = symbols->declare(node->value, ObjectKind::Constant, node->inferredType, 1, adr);

    if (node->tabIndex == -1) {
        reportError("Redeclaration of identifier '" + node->value + "'");
    } else {
        symbols->tabAt(node->tabIndex).initialized = true;
    }
}

void SemanticAnalyzer::visitTypeDecl(ASTNode* node) {
    annotate(node);
    TypeInfo type = node->children.empty() ? TypeInfo{} : resolveType(node->children.front());
    node->inferredType = type.type;
    node->tabIndex = symbols->declare(node->value, ObjectKind::Type, type.type, 1, 0);

    if (node->tabIndex == -1) {
        reportError("Redeclaration of identifier '" + node->value + "'");
        return;
    }

    if (!node->children.empty() && node->children.front()->kind == ASTKind::EnumType) {
        int ordinal = 0;
        std::set<std::string> seen;

        for (ASTNode* enumerator : node->children.front()->children) {
            if (!enumerator) {
                continue;
            }

            std::string key = normalize(enumerator->value);
            if (seen.count(key)) {
                reportError("Duplicate enumerated identifier '" + enumerator->value + "'");
                continue;
            }
            seen.insert(key);

            enumerator->inferredType = type.type;
            enumerator->tabIndex = symbols->declare(enumerator->value, ObjectKind::Constant, type.type, 1, ordinal++);
            annotate(enumerator);

            if (enumerator->tabIndex == -1) {
                reportError("Redeclaration of identifier '" + enumerator->value + "'");
            } else {
                symbols->tabAt(enumerator->tabIndex).initialized = true;
            }
        }
    }
}

void SemanticAnalyzer::visitVarDecl(ASTNode* node) {
    annotate(node);
    TypeInfo type = node->children.empty() ? TypeInfo{} : resolveType(node->children.front());
    node->inferredType = type.type;

    ObjectKind obj = node->kind == ASTKind::FieldDecl ? ObjectKind::Field : ObjectKind::Variable;
    node->tabIndex = symbols->declare(node->value, obj, type.type);

    if (node->tabIndex == -1) {
        reportError("Redeclaration of identifier '" + node->value + "'");
    }
}

void SemanticAnalyzer::visitProcDecl(ASTNode* node) {
    annotate(node);
    SemanticType type = makeType(TypeKind::Procedure);
    node->inferredType = type;
    node->tabIndex = symbols->declare(node->value, ObjectKind::Procedure, type, 1, 0);

    if (node->tabIndex == -1) {
        reportError("Redeclaration of identifier '" + node->value + "'");
        return;
    }

    int blockIndex = symbols->enterBlock();
    symbols->tabAt(node->tabIndex).ref = blockIndex;
    node->blockIndex = blockIndex;
    node->lexicalLevel = symbols->currentLevel();

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

    symbols->leaveBlock();
}

void SemanticAnalyzer::visitFuncDecl(ASTNode* node) {
    annotate(node);
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

    node->inferredType = returnType.type;
    node->tabIndex = symbols->declare(node->value, ObjectKind::Function, returnType.type, 1, 0);

    if (node->tabIndex == -1) {
        reportError("Redeclaration of identifier '" + node->value + "'");
        return;
    }

    int blockIndex = symbols->enterBlock();
    symbols->tabAt(node->tabIndex).ref = blockIndex;
    node->blockIndex = blockIndex;
    node->lexicalLevel = symbols->currentLevel();

    if (parameterPart) {
        visitDeclarationPart(parameterPart);
    }

    if (body) {
        visitBlock(body);
    }

    if (!body || !containsReturnAssignment(body, node->value)) {
        reportError("Function '" + node->value + "' does not assign a return value");
    }

    symbols->leaveBlock();
}

void SemanticAnalyzer::visitParam(ASTNode* node) {
    annotate(node);
    TypeInfo type = node->children.empty() ? TypeInfo{} : resolveType(node->children.front());
    node->inferredType = type.type;
    node->tabIndex = symbols->declare(node->value, ObjectKind::Parameter, type.type);

    if (node->tabIndex == -1) {
        reportError("Redeclaration of identifier '" + node->value + "'");
    }
}

SemanticAnalyzer::TypeInfo SemanticAnalyzer::resolveType(ASTNode* node) {
    if (!node) {
        return TypeInfo{};
    }

    annotate(node);

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
    int index = symbols->lookup(node->value);

    if (index == -1 || symbols->tabAt(index).obj != ObjectKind::Type) {
        reportError("Unknown type '" + node->value + "'");
        type.type = makeUnknownType();
        node->inferredType = type.type;
        return type;
    }

    const TabEntry& entry = symbols->tabAt(index);
    type.type = entry.type;
    node->inferredType = type.type;
    node->tabIndex = index;
    return type;
}

SemanticAnalyzer::TypeInfo SemanticAnalyzer::resolveRangeType(ASTNode* node) {
    TypeInfo type;

    if (node->children.size() < 2) {
        reportError("Invalid subrange type");
        type.type = makeUnknownType();
        node->inferredType = type.type;
        return type;
    }

    ConstantInfo low = evaluateConstant(node->children[0]);
    ConstantInfo high = evaluateConstant(node->children[1]);

    if (!low.valid || !high.valid) {
        reportError("Invalid constant in subrange type");
        type.type = makeUnknownType();
        node->inferredType = type.type;
        return type;
    }

    if (low.type.kind == TypeKind::Real || high.type.kind == TypeKind::Real) {
        reportError("Subrange cannot use Real type");
        type.type = makeUnknownType();
        node->inferredType = type.type;
        return type;
    }

    if (low.type.kind != high.type.kind) {
        reportError("Subrange bounds must have the same type");
        type.type = makeUnknownType();
        node->inferredType = type.type;
        return type;
    }

    if (low.hasOrdinal && high.hasOrdinal && low.ordinal > high.ordinal) {
        reportError("Invalid range: lower bound greater than upper bound");
        type.type = makeUnknownType();
        node->inferredType = type.type;
        return type;
    }

    type.type = makeType(TypeKind::Subrange);
    type.low = low.ordinal;
    type.high = high.ordinal;
    type.hasBounds = low.hasOrdinal && high.hasOrdinal;
    node->inferredType = type.type;
    return type;
}

SemanticAnalyzer::TypeInfo SemanticAnalyzer::resolveArrayType(ASTNode* node) {
    TypeInfo type;
    type.type = makeType(TypeKind::Array);

    if (node->children.size() < 2) {
        reportError("Invalid array type");
        type.type = makeUnknownType();
        node->inferredType = type.type;
        return type;
    }

    TypeInfo indexType = resolveType(node->children[0]);
    TypeInfo elementType = resolveType(node->children[1]);

    if (indexType.type.kind == TypeKind::Real) {
        reportError("Array index type cannot be Real");
    }

    if (!isOrdinal(indexType.type)) {
        reportError("Array index type must be ordinal");
    }

    ATabEntry arrayEntry;
    arrayEntry.xtyp = indexType.type;
    arrayEntry.etyp = elementType.type;
    arrayEntry.eref = elementType.type.ref;
    arrayEntry.low = indexType.low;
    arrayEntry.high = indexType.high;
    arrayEntry.elsz = getTypeSize(elementType.type);
    arrayEntry.size = indexType.hasBounds && indexType.high >= indexType.low
        ? (indexType.high - indexType.low + 1) * (arrayEntry.elsz > 0 ? arrayEntry.elsz : 1)
        : 0;

    type.type.ref = symbols->addArrayType(arrayEntry);
    node->inferredType = type.type;
    return type;
}

SemanticAnalyzer::TypeInfo SemanticAnalyzer::resolveRecordType(ASTNode* node) {
    TypeInfo type;
    type.type = makeType(TypeKind::Record);
    std::set<std::string> fields;

    int blockIndex = symbols->enterBlock();

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
        visitVarDecl(field);
    }

    type.type.ref = blockIndex;
    node->blockIndex = blockIndex;
    node->lexicalLevel = symbols->currentLevel();
    node->inferredType = type.type;
    symbols->leaveBlock();
    return type;
}

SemanticAnalyzer::TypeInfo SemanticAnalyzer::resolveEnumType(ASTNode* node) {
    TypeInfo type;
    type.type = makeType(TypeKind::Enumerated);
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

    annotate(node);

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
            constant.type = makeRealType();
        } else if (numeric) {
            constant.type = makeIntType();
            constant.ordinal = std::atoi(node->value.c_str());
            constant.hasOrdinal = true;
        } else if (node->value.size() == 1) {
            constant.type = makeCharType();
            constant.ordinal = static_cast<unsigned char>(node->value[0]);
            constant.hasOrdinal = true;
        } else {
            constant.type = makeStringType();
        }

        node->inferredType = constant.type;
        return constant;
    }

    if (node->kind == ASTKind::Var) {
        int index = symbols->lookup(node->value);

        if (index == -1 || symbols->tabAt(index).obj != ObjectKind::Constant) {
            reportError("Undeclared constant '" + node->value + "'");
            return constant;
        }

        const TabEntry& entry = symbols->tabAt(index);
        constant.type = entry.type;
        constant.ordinal = entry.adr;
        constant.hasOrdinal = isOrdinal(entry.type);
        constant.valid = true;
        node->tabIndex = index;
        node->inferredType = constant.type;
        return constant;
    }

    return constant;
}

bool SemanticAnalyzer::isRecordFieldContext(ASTNode* node) const {
    return node && node->kind == ASTKind::FieldDecl;
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

bool SemanticAnalyzer::isOrdinal(const SemanticType& type) const {
    return type.kind == TypeKind::Integer ||
           type.kind == TypeKind::Char ||
           type.kind == TypeKind::Boolean ||
           type.kind == TypeKind::Enumerated ||
           type.kind == TypeKind::Subrange;
}

std::string SemanticAnalyzer::normalize(const std::string& value) const {
    std::string result = value;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return result;
}

void SemanticAnalyzer::reportError(const std::string& message) {
    semanticErrors.push_back(SemanticError{message});
}
