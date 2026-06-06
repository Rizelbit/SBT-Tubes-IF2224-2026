#include "IntermediateCode.hpp"

#include <iomanip>
#include <stdexcept>

std::string opcodeToString(OpCode op) {
    switch (op) {
        case OpCode::INT: return "INT";
        case OpCode::LIT: return "LIT";
        case OpCode::LOD: return "LOD";
        case OpCode::STO: return "STO";
        case OpCode::OPR: return "OPR";
        case OpCode::JMP: return "JMP";
        case OpCode::JPC: return "JPC";
        case OpCode::CAL: return "CAL";
        case OpCode::RET: return "RET";
    }
    return "???";
}

int CodeBuffer::emit(OpCode op, int level, int operand, const std::string& comment) {
    int address = currentAddress();
    code.push_back(Instruction{op, level, operand, comment});
    return address;
}

void CodeBuffer::patch(int index, int operand) {
    if (index < 0 || index >= static_cast<int>(code.size())) {
        throw std::out_of_range("Invalid intermediate code patch address");
    }
    code[index].operand = operand;
}

int CodeBuffer::currentAddress() const {
    return static_cast<int>(code.size());
}

const std::vector<Instruction>& CodeBuffer::instructions() const {
    return code;
}

void CodeBuffer::print(std::ostream& out) const {
    for (int i = 0; i < static_cast<int>(code.size()); ++i) {
        const Instruction& instruction = code[i];
        out << i
            << " " << opcodeToString(instruction.op)
            << " " << instruction.level
            << " " << instruction.operand;
        if (!instruction.comment.empty()) {
            out << " ; " << instruction.comment;
        }
        out << "\n";
    }
}
