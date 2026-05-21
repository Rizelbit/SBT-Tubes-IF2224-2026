#pragma once
#include <string>
#include "TypeSystem.hpp"

namespace SemanticErrors {
    std::string undeclaredIdentifier(const std::string& name);
    std::string typeMismatchAssignment(const SemanticType& target, const SemanticType& value);
    std::string operatorMismatch(const std::string& op, const SemanticType& left, const SemanticType& right);
    std::string operatorMismatchUnary(const std::string& op, const SemanticType& operand);
    std::string conditionNotBoolean(const std::string& context);
    std::string notAnArray(const std::string& name);
    std::string invalidArrayIndex(const SemanticType& expected, const SemanticType& actual);
    std::string notARecord(const std::string& name);
    std::string invalidRecordField(const std::string& recordName, const std::string& fieldName);
    std::string notAProcedure(const std::string& name);
    std::string notAFunction(const std::string& name);
    std::string invalidArgumentCount(const std::string& name, int expected, int actual);
    std::string invalidArgumentType(const std::string& name, int paramIndex, const SemanticType& expected, const SemanticType& actual);
    std::string invalidForControlVar(const std::string& name);
}
