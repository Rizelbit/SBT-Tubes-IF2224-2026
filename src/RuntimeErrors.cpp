#include "RuntimeErrors.hpp"

std::string runtimeErrorTypeToString(RuntimeErrorType type) {
    switch (type) {
        case RuntimeErrorType::StackOverflow:        return "Stack Overflow";
        case RuntimeErrorType::StackUnderflow:       return "Stack Underflow";
        case RuntimeErrorType::InvalidJump:          return "Invalid Jump";
        case RuntimeErrorType::InvalidMemoryAccess:  return "Invalid Memory Access";
        case RuntimeErrorType::DivisionByZero:       return "Division By Zero";
        case RuntimeErrorType::InvalidCallFrame:     return "Invalid Call Frame";
        case RuntimeErrorType::ExecutionStepLimit:   return "Execution Step Limit Exceeded";
        case RuntimeErrorType::InvalidInstruction:   return "Invalid Instruction";
        case RuntimeErrorType::Unknown:              return "Unknown Runtime Error";
    }
    return "Unknown";
}

void RuntimeErrorManager::addError(RuntimeErrorType type, const std::string& message, int address) {
    errors_.push_back({type, message, address});
}

const std::vector<RuntimeError>& RuntimeErrorManager::errors() const {
    return errors_;
}

bool RuntimeErrorManager::hasErrors() const {
    return !errors_.empty();
}

void RuntimeErrorManager::clear() {
    errors_.clear();
}
