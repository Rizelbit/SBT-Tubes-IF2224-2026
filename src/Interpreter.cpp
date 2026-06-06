#include "Interpreter.hpp"

#include <iostream>
#include <sstream>
#include <cmath>
#include <limits>

Interpreter::Interpreter(const CodeBuffer& code, const SymbolTable* sym, const std::unordered_map<int, int>* addrToTabIndex) : code_(code), sym_(sym), addrToTabIndex_(addrToTabIndex)
{
    stack_.reserve(MAX_STACK);
}

bool Interpreter::run() {
    pc_ = 0;
    sp_ = -1;
    bp_ = 0;
    output_.clear();
    errorMgr_.clear();
    steps_ = 0;

    const auto& instrs = code_.instructions();

    while (static_cast<size_t>(pc_) < instrs.size()) {
        if (steps_++ >= MAX_STEPS) {
            errorMgr_.addError(RuntimeErrorType::ExecutionStepLimit,
                "Execution step limit exceeded", pc_);
            return false;
        }

        const Instruction& instr = instrs[pc_];
        pc_++;
        executeInstruction(instr);

        if (errorMgr_.hasErrors()) {
            return false;
        }
    }

    return true;
}

const std::string& Interpreter::output() const {
    return output_;
}

const std::vector<RuntimeError>& Interpreter::errors() const {
    return errorMgr_.errors();
}

bool Interpreter::hasErrors() const {
    return errorMgr_.hasErrors();
}

void Interpreter::push(const Value& v) {
    if (sp_ + 1 >= MAX_STACK) {
        errorMgr_.addError(RuntimeErrorType::StackOverflow,
            "Stack overflow on push", pc_ - 1);
        return;
    }
    sp_++;
    if (static_cast<size_t>(sp_) >= stack_.size()) {
        stack_.resize(sp_ + 1, 0);
    }
    stack_[sp_] = v;
}

Value Interpreter::pop() {
    if (sp_ < 0) {
        errorMgr_.addError(RuntimeErrorType::StackUnderflow,
            "Stack underflow on pop", pc_ - 1);
        return 0;
    }
    return stack_[sp_--];
}

Value& Interpreter::top() {
    if (sp_ < 0) {
        static Value dummy = 0;
        errorMgr_.addError(RuntimeErrorType::StackUnderflow,
            "Stack underflow accessing top", pc_ - 1);
        return dummy;
    }
    if (static_cast<size_t>(sp_) >= stack_.size()) {
        stack_.resize(sp_ + 1, 0);
    }
    return stack_[sp_];
}

int Interpreter::base(int level) const {
    int b = bp_;
    while (level > 0) {
        if (b < 0 || static_cast<size_t>(b) >= stack_.size()) {
            return 0;
        }
        b = toInt(stack_[b]);
        level--;
    }
    return b;
}

bool Interpreter::checkStackBounds(int requiredFreeSlots) {
    if (sp_ + requiredFreeSlots >= MAX_STACK) {
        errorMgr_.addError(RuntimeErrorType::StackOverflow,
            "Stack overflow: need " + std::to_string(requiredFreeSlots) + " slots", pc_ - 1);
        return false;
    }
    return true;
}

bool Interpreter::checkStackHasElements(int required) {
    if (sp_ + 1 < required) {
        errorMgr_.addError(RuntimeErrorType::StackUnderflow,
            "Stack underflow: need " + std::to_string(required) + " elements", pc_ - 1);
        return false;
    }
    return true;
}

bool Interpreter::checkAddress(int addr) {
    if (addr < 0 || static_cast<size_t>(addr) >= stack_.size()) {
        errorMgr_.addError(RuntimeErrorType::InvalidMemoryAccess,
            "Invalid memory access at address " + std::to_string(addr), pc_ - 1);
        return false;
    }
    return true;
}

void Interpreter::emitOutput(const std::string& s) {
    output_ += s;
}

void Interpreter::emitOutputLine(const std::string& s) {
    output_ += s + "\n";
}

void Interpreter::executeInstruction(const Instruction& instr) {
    switch (instr.op) {
        case OpCode::INT: {
            if (!checkStackBounds(instr.operand)) return;
            int oldSp = sp_;
            sp_ += instr.operand;
            if (sp_ >= static_cast<int>(stack_.size())) {
                stack_.resize(sp_ + 1, 0);
            }
            // initiate slot baru dengan 0
            for (int i = oldSp + 1; i <= sp_; ++i) {
                stack_[i] = 0;
            }
            break;
        }
        case OpCode::LIT: {
            if (instr.operand == -1 && !instr.comment.empty()) {
                // encoded dalam "string : ..."
                std::string s = instr.comment;
                const std::string prefix = "string:";
                if (s.find(prefix) == 0) {
                    s = s.substr(prefix.size());
                }
                push(s);
            } else {
                push(instr.operand);
            }
            break;
        }
        case OpCode::LOD: {
            if (instr.operand == -1) {
                // pop, lalu load
                if (!checkStackHasElements(1)) return;
                int addr = toInt(pop());
                if (!checkAddress(addr)) return;
                push(stack_[addr]);
            } else {
                int addr = base(instr.level) + instr.operand;
                if (!checkAddress(addr)) return;
                push(stack_[addr]);
            }
            break;
        }
        case OpCode::STO: {
            if (instr.operand == -1) {
                // pop addr, pop value, lalu store
                if (!checkStackHasElements(2)) return;
                int addr = toInt(pop());
                Value val = pop();
                if (!checkAddress(addr)) return;
                stack_[addr] = val;
            } else {
                if (!checkStackHasElements(1)) return;
                int addr = base(instr.level) + instr.operand;
                if (!checkAddress(addr)) return;
                stack_[addr] = pop();
            }
            break;
        }
        case OpCode::OPR: {
            executeOPR(instr.operand);
            break;
        }
        case OpCode::JMP: {
            if (instr.operand < 0 || instr.operand >= static_cast<int>(code_.instructions().size())) {
                errorMgr_.addError(RuntimeErrorType::InvalidJump,
                    "Invalid jump target " + std::to_string(instr.operand), pc_ - 1);
                return;
            }
            pc_ = instr.operand;
            break;
        }
        case OpCode::JPC: {
            if (!checkStackHasElements(1)) return;
            Value cond = pop();
            if (!isTruthy(cond)) {
                if (instr.operand < 0 || instr.operand >= static_cast<int>(code_.instructions().size())) {
                    errorMgr_.addError(RuntimeErrorType::InvalidJump,
                        "Invalid JPC target " + std::to_string(instr.operand), pc_ - 1);
                    return;
                }
                pc_ = instr.operand;
            }
            break;
        }
        case OpCode::CAL: {
            if (!sym_ || !addrToTabIndex_) {
                if (!checkStackBounds(3)) return;
                Value sl = base(instr.level);
                Value dl = bp_;
                Value ra = pc_;
                push(sl);
                push(dl);
                push(ra);
                bp_ = sp_ - 2; // alamat si SL
                pc_ = instr.operand;
                break;
            }

            int tabIdx = -1;
            auto it = addrToTabIndex_->find(instr.operand);
            if (it != addrToTabIndex_->end()) {
                tabIdx = it->second;
            }

            int psze = 0;
            if (tabIdx >= 0 && tabIdx < sym_->tabSize()) {
                const TabEntry& entry = sym_->tabAt(tabIdx);
                int bref = entry.ref;
                if (bref > 0 && bref < sym_->btabSize()) {
                    psze = sym_->btabAt(bref).psze;
                }
            }

            if (!checkStackHasElements(psze)) return;
            std::vector<Value> args;
            args.reserve(psze);
            for (int i = 0; i < psze; ++i) {
                args.push_back(stack_[sp_ - psze + 1 + i]);
            }
            sp_ -= psze;

            if (!checkStackBounds(3 + psze)) return;
            push(base(instr.level));
            push(bp_);
            push(pc_);
            for (auto& arg : args) {
                push(arg);
            }
            bp_ = sp_ - psze - 2;
            pc_ = instr.operand;
            break;
        }
        case OpCode::RET: {
            if (bp_ == 0) {
                // akhir program
                pc_ = static_cast<int>(code_.instructions().size());
                break;
            }
            if (bp_ < 0 || bp_ + 2 >= static_cast<int>(stack_.size())) {
                errorMgr_.addError(RuntimeErrorType::InvalidCallFrame,
                    "Invalid call frame on RET", pc_ - 1); // invalid 
                return;
            }
            int oldPc = toInt(stack_[bp_ + 2]);
            int oldBp = toInt(stack_[bp_ + 1]);
            sp_ = bp_ - 1;
            bp_ = oldBp;
            pc_ = oldPc;
            break;
        }
        default: {
            errorMgr_.addError(RuntimeErrorType::InvalidInstruction,
                "Unknown opcode", pc_ - 1);
            break;
        }
    }
}

void Interpreter::executeOPR(int operand) {
    switch (operand) {
        case 0: { // no-op / internal ret
            break;
        }
        case 1: { // NEG
            if (!checkStackHasElements(1)) return;
            stack_[sp_] = negValue(stack_[sp_]);
            break;
        }
        case 2: { // ADD
            if (!checkStackHasElements(2)) return;
            Value b = pop();
            Value a = pop();
            push(addValues(a, b));
            break;
        }
        case 3: { // SUB
            if (!checkStackHasElements(2)) return;
            Value b = pop();
            Value a = pop();
            push(subValues(a, b));
            break;
        }
        case 4: { // MUL
            if (!checkStackHasElements(2)) return;
            Value b = pop();
            Value a = pop();
            push(mulValues(a, b));
            break;
        }
        case 5: { // DIV
            if (!checkStackHasElements(2)) return;
            Value b = pop();
            Value a = pop();
            try {
                push(divValues(a, b));
            } catch (const std::exception& e) {
                errorMgr_.addError(RuntimeErrorType::DivisionByZero,
                    e.what(), pc_ - 1);
            }
            break;
        }
        case 6: { // MOD
            if (!checkStackHasElements(2)) return;
            Value b = pop();
            Value a = pop();
            try {
                push(modValues(a, b));
            } catch (const std::exception& e) {
                errorMgr_.addError(RuntimeErrorType::DivisionByZero,
                    e.what(), pc_ - 1);
            }
            break;
        }
        case 7: { // EQL
            if (!checkStackHasElements(2)) return;
            Value b = pop();
            Value a = pop();
            push(eqValues(a, b));
            break;
        }
        case 8: { // NEQ
            if (!checkStackHasElements(2)) return;
            Value b = pop();
            Value a = pop();
            push(neqValues(a, b));
            break;
        }
        case 9: { // LSS
            if (!checkStackHasElements(2)) return;
            Value b = pop();
            Value a = pop();
            push(lssValues(a, b));
            break;
        }
        case 10: { // GEQ
            if (!checkStackHasElements(2)) return;
            Value b = pop();
            Value a = pop();
            push(geqValues(a, b));
            break;
        }
        case 11: { // GTR
            if (!checkStackHasElements(2)) return;
            Value b = pop();
            Value a = pop();
            push(gtrValues(a, b));
            break;
        }
        case 12: { // LEQ
            if (!checkStackHasElements(2)) return;
            Value b = pop();
            Value a = pop();
            push(leqValues(a, b));
            break;
        }
        case 13: { // WRT
            if (!checkStackHasElements(1)) return;
            emitOutput(toString(pop()));
            break;
        }
        case 14: { // WRTLN
            if (!checkStackHasElements(1)) return;
            emitOutputLine(toString(pop()));
            break;
        }
        case 15: { // AND
            if (!checkStackHasElements(2)) return;
            Value b = pop();
            Value a = pop();
            push(andValues(a, b));
            break;
        }
        case 16: { // OR
            if (!checkStackHasElements(2)) return;
            Value b = pop();
            Value a = pop();
            push(orValues(a, b));
            break;
        }
        case 17: { // NOT
            if (!checkStackHasElements(1)) return;
            stack_[sp_] = notValue(stack_[sp_]);
            break;
        }
        case 18: { // READ
            if (!checkStackHasElements(2)) return;
            {
                int addr = toInt(pop());
                int level = toInt(pop());
                int target = base(level) + addr;
                if (!checkAddress(target)) return;
                int val = 0;
                if (std::cin >> val) {
                    stack_[target] = val;
                } else {
                    stack_[target] = 0;
                }
            }
            break;
        }
        case 19: { // READLN
            if (!checkStackHasElements(2)) return;
            {
                int addr = toInt(pop());
                int level = toInt(pop());
                int target = base(level) + addr;
                if (!checkAddress(target)) return;
                int val = 0;
                if (std::cin >> val) {
                    stack_[target] = val;
                } else {
                    stack_[target] = 0;
                }
            }
            break;
        }
        default: {
            errorMgr_.addError(RuntimeErrorType::InvalidInstruction,
                "Unknown OPR operand " + std::to_string(operand), pc_ - 1);
            break;
        }
    }
}
