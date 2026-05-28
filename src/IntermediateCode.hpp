#pragma once

#include <ostream>
#include <string>
#include <vector>

enum class OpCode {
    INT,
    LIT,
    LOD,
    STO,
    OPR,
    JMP,
    JPC,
    CAL,
    RET,
    HALT
};

struct Instruction {
    OpCode op;
    int level = 0;
    int operand = 0;
    std::string comment;
};

std::string opcodeToString(OpCode op);

class CodeBuffer {
public:
    int emit(OpCode op, int level, int operand, const std::string& comment = "");
    void patch(int index, int operand);
    int currentAddress() const;
    const std::vector<Instruction>& instructions() const;
    void print(std::ostream& out) const;

private:
    std::vector<Instruction> code;
};
