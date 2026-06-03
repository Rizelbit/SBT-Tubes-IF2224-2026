#pragma once

#include <string>
#include <vector>

enum class RuntimeErrorType {
    StackOverflow,
    StackUnderflow,
    InvalidJump,
    InvalidMemoryAccess,
    DivisionByZero,
    InvalidCallFrame,
    ExecutionStepLimit,
    InvalidInstruction,
    Unknown
};

struct RuntimeError {
    RuntimeErrorType type;
    std::string message;
    int instructionAddress;
};

class RuntimeErrorManager {
public:
    void addError(RuntimeErrorType type, const std::string& message, int address);
    const std::vector<RuntimeError>& errors() const;
    bool hasErrors() const;
    void clear();

private:
    std::vector<RuntimeError> errors_;
};

std::string runtimeErrorTypeToString(RuntimeErrorType type);
