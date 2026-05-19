#include "TypeSystem.hpp"
#include <algorithm>
#include <cctype>

std::string typeKindToString(TypeKind k) {
    switch (k) {
        case TypeKind::Unknown:    return "Unknown";
        case TypeKind::Void:       return "Void";
        case TypeKind::Integer:    return "Integer";
        case TypeKind::Real:       return "Real";
        case TypeKind::Char:       return "Char";
        case TypeKind::Boolean:    return "Boolean";
        case TypeKind::String:     return "String";
        case TypeKind::Subrange:   return "Subrange";
        case TypeKind::Enumerated: return "Enumerated";
        case TypeKind::Array:      return "Array";
        case TypeKind::Record:     return "Record";
        case TypeKind::Procedure:  return "Procedure";
        case TypeKind::Function:   return "Function";
    }
    return "Unknown";
}

std::string typeToString(const SemanticType& t) {
    std::string s = typeKindToString(t.kind);
    if (t.ref != 0)
        s += "[ref=" + std::to_string(t.ref) + "]";
    return s;
}


bool isNumericType(const SemanticType& t) {
    return t.kind == TypeKind::Integer || t.kind == TypeKind::Real;
}


bool isOrdinalType(const SemanticType& t) {
    return t.kind == TypeKind::Integer || t.kind == TypeKind::Char || t.kind == TypeKind::Boolean || t.kind == TypeKind::Enumerated || t.kind == TypeKind::Subrange;
}


bool isCompatible(const SemanticType& left, const SemanticType& right) {

    if (left.kind == TypeKind::Unknown || right.kind == TypeKind::Unknown)
        return false;

    
    if (isNumericType(left) && isNumericType(right))
        return true;

    if (left.kind == TypeKind::Subrange && right.kind == TypeKind::Integer) return true;
    if (left.kind == TypeKind::Integer && right.kind == TypeKind::Subrange) return true;

    if (left.kind == TypeKind::Subrange && right.kind == TypeKind::Subrange) return true;

    return left.kind == right.kind && left.ref == right.ref;
}

bool isAssignmentCompatible(const SemanticType& target, const SemanticType& value) {
    if (target.kind == TypeKind::Unknown || value.kind == TypeKind::Unknown) return false;

    if (target.kind == TypeKind::Real && value.kind == TypeKind::Integer) return true;

    if (target.kind == TypeKind::Subrange && value.kind == TypeKind::Integer) return true;
    if (target.kind == TypeKind::Integer && value.kind == TypeKind::Subrange) return true;
    if (target.kind == TypeKind::Subrange && value.kind == TypeKind::Subrange) return true;

    return target.kind == value.kind && target.ref == value.ref;
}

static bool isRelationalOp(const std::string& op) {
    return op == "=" || op == "<>" || op == "<" || op == ">" || op == "<=" || op == ">=";
}


SemanticType inferBinary(const std::string& op, const SemanticType& left, const SemanticType& right) {
   
    // jika salahs atu variable unknown, makas hasilnya jg unknown
    if (left.kind == TypeKind::Unknown || right.kind == TypeKind::Unknown)return makeUnknownType();

    // artimatika
    if (op == "+") {
        if (left.kind == TypeKind::String && right.kind == TypeKind::String)
            return makeStringType();
        if (left.kind == TypeKind::Integer && right.kind == TypeKind::Integer)
            return makeIntType();
        if (isNumericType(left) && isNumericType(right))
            return makeRealType();
        return makeUnknownType();
    }

    if (op == "-" || op == "*") {
        if (left.kind == TypeKind::Integer && right.kind == TypeKind::Integer)
            return makeIntType();
        if (isNumericType(left) && isNumericType(right))
            return makeRealType();
        return makeUnknownType();
    }

    if (op == "/") {
        if (isNumericType(left) && isNumericType(right))
            return makeRealType();
        return makeUnknownType();
    }

    if (op == "div") {
        if (left.kind == TypeKind::Integer && right.kind == TypeKind::Integer)
            return makeIntType();
        return makeUnknownType();
    }

    if (op == "mod") {
        if (left.kind == TypeKind::Integer && right.kind == TypeKind::Integer)
            return makeIntType();
        return makeUnknownType();
    }

    // logika
    if (op == "and" || op == "or") {
        if (left.kind == TypeKind::Boolean && right.kind == TypeKind::Boolean) return makeBoolType();
        return makeUnknownType();
    }


    if (isRelationalOp(op)) {
        if (isCompatible(left, right)) return makeBoolType();
        return makeUnknownType();
    }

    return makeUnknownType();
}


SemanticType inferUnary(const std::string& op, const SemanticType& operand) {
    if (operand.kind == TypeKind::Unknown)
        return makeUnknownType();

    if (op == "not") {
        if (operand.kind == TypeKind::Boolean)
            return makeBoolType();
        return makeUnknownType();
    }

    if (op == "-" || op == "+") {
        if (operand.kind == TypeKind::Integer) return makeIntType();
        if (operand.kind == TypeKind::Real) return makeRealType();
        return makeUnknownType();
    }

    return makeUnknownType();
}