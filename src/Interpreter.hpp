#pragma once

#include "IntermediateCode.hpp"
#include "RuntimeValue.hpp"
#include "RuntimeErrors.hpp"
#include "SymbolTable.hpp"
#include <string>
#include <vector>
#include <unordered_map>

class Interpreter {
public:
    Interpreter(const CodeBuffer& code,
                const SymbolTable* sym = nullptr,
                const std::unordered_map<int, int>* addrToTabIndex = nullptr);

    bool run();

    const std::string& output() const;
    const std::vector<RuntimeError>& errors() const;
    bool hasErrors() const;

private:
    const CodeBuffer& code_;
    const SymbolTable* sym_;
    const std::unordered_map<int, int>* addrToTabIndex_;

    // VM state
    int pc_ = 0;
    int sp_ = -1;
    int bp_ = 0;
    std::vector<Value> stack_;

    std::string output_;
    RuntimeErrorManager errorMgr_;

    // Constants
    static constexpr int MAX_STACK = 10000;
    static constexpr long long MAX_STEPS = 1000000LL;

    long long steps_ = 0;

    // Helpers
    void push(const Value& v);
    Value pop();
    Value& top();
    int base(int level) const;
    bool checkStackBounds(int requiredFreeSlots);
    bool checkStackHasElements(int required);
    bool checkAddress(int addr);
    void emitOutput(const std::string& s);
    void emitOutputLine(const std::string& s);

    // Execution
    void executeInstruction(const Instruction& instr);

    // OPR
    void executeOPR(int operand);
};
