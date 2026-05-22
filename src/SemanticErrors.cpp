#include "SemanticErrors.hpp"

namespace SemanticErrors {

std::string undeclaredIdentifier(const std::string& name) {
    return "Undeclared identifier '" + name + "'";
}

std::string typeMismatchAssignment(const SemanticType& target, const SemanticType& value) {
    return "Type mismatch: cannot assign " + typeToString(value) + " to " + typeToString(target);
}

std::string operatorMismatch(const std::string& op, const SemanticType& left, const SemanticType& right) {
    return "Operator '" + op + "' cannot be applied to " + typeToString(left) + " and " + typeToString(right);
}

std::string operatorMismatchUnary(const std::string& op, const SemanticType& operand) {
    return "Operator '" + op + "' cannot be applied to " + typeToString(operand);
}

std::string conditionNotBoolean(const std::string& context) {
    return "Condition in " + context + " must be Boolean";
}

std::string notAnArray(const std::string& name) {
    return "Identifier '" + name + "' is not an array";
}

std::string invalidArrayIndex(const SemanticType& expected, const SemanticType& actual) {
    return "Invalid array index: expected " + typeToString(expected) + ", got " + typeToString(actual);
}

std::string notARecord(const std::string& name) {
    return "Identifier '" + name + "' is not a record";
}

std::string invalidRecordField(const std::string& recordName, const std::string& fieldName) {
    return "Record '" + recordName + "' has no field '" + fieldName + "'";
}

std::string notAProcedure(const std::string& name) {
    return "Identifier '" + name + "' is not a procedure";
}

std::string notAFunction(const std::string& name) {
    return "Identifier '" + name + "' is not a function";
}

std::string invalidArgumentCount(const std::string& name, int expected, int actual) {
    return "Invalid number of arguments for '" + name + "': expected " + std::to_string(expected) + ", got " + std::to_string(actual);
}

std::string invalidArgumentType(const std::string& name, int paramIndex, const SemanticType& expected, const SemanticType& actual) {
    return "Invalid argument type for parameter " + std::to_string(paramIndex) + " of '" + name + "': expected " + typeToString(expected) + ", got " + typeToString(actual);
}

std::string invalidForControlVar(const std::string& name) {
    return "Invalid control variable '" + name + "' for 'for' loop (must be ordinal type and properly declared)";
}

} // namespace SemanticErrors
