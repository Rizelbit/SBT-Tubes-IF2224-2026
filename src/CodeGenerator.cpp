#include "CodeGenerator.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>

namespace OPR {
    constexpr int NEG = 1;
    constexpr int ADD = 2;
    constexpr int SUB = 3;
    constexpr int MUL = 4;
    constexpr int DIV = 5;
    constexpr int MOD = 6;
    constexpr int EQL = 7;
    constexpr int NEQ = 8;
    constexpr int LSS = 9;
    constexpr int GEQ = 10;
    constexpr int GTR = 11;
    constexpr int LEQ = 12;
    constexpr int WRT = 13;
    constexpr int WRTLN = 14;
    constexpr int AND = 15;
    constexpr int OR = 16;
    constexpr int NOT = 17;
    constexpr int READ = 18;
    constexpr int READLN = 19;
}


CodeGenerator::CodeGenerator(const SymbolTable& symbols)
    : sym_(symbols)
    , currentLevel_(0)
{}


CodeBuffer CodeGenerator::generate(ASTNode* root) {
    buf_ = CodeBuffer{};
    if (root) {
        generateNode(root);
    }
    return buf_;
}


void CodeGenerator::generateNode(ASTNode* node) {
    if (!node) return;

    switch (node->kind) {
        case ASTKind::Program:
            generateProgram(node);
            break;
        case ASTKind::Block:
            generateBlock(node);
            break;
        case ASTKind::DeclarationPart:
            generateDeclarationPart(node);
            break;
        case ASTKind::Assign:
            generateAssign(node);
            break;
        case ASTKind::If:
            generateIf(node);
            break;
        case ASTKind::While:
            generateWhile(node);
            break;
        case ASTKind::Repeat:
            generateRepeat(node);
            break;
        case ASTKind::For:
            generateFor(node);
            break;
        case ASTKind::Case:
            generateCase(node);
            break;
        case ASTKind::ProcedureCall:
            generateProcedureCall(node);
            break;
        case ASTKind::FunctionCall:
            generateFunctionCall(node);
            break;
        case ASTKind::ProcDecl:
        case ASTKind::FuncDecl:
            generateSubprogramDecl(node);
            break;

    
        case ASTKind::BinOp:
        case ASTKind::UnaryOp:
        case ASTKind::Literal:
        case ASTKind::Var:
        case ASTKind::ArrayAccess:
        case ASTKind::FieldAccess:
            generateExpression(node);
            break;

        case ASTKind::Empty:
            break; 

        default:
            break;
    }
}


void CodeGenerator::generateProgram(ASTNode* node) {
    int frameSize = FRAME_HEADER;

    int maxEndAdr = 0;
    for (int i = 0; i < sym_.tabSize(); ++i) {
        const TabEntry& e = sym_.tabAt(i);
        if (e.lev == 0 &&
            (e.obj == ObjectKind::Variable || e.obj == ObjectKind::Parameter)) {
            int endAdr = e.adr + typeSize(e.type);
            if (endAdr > maxEndAdr) maxEndAdr = endAdr;
        }
    }

    frameSize += maxEndAdr;
    buf_.emit(OpCode::INT, 0, frameSize, "alokasi frame program");

    for (ASTNode* child : node->children) {
        generateNode(child);
    }

    buf_.emit(OpCode::RET, 0, 0, "akhir program");
}


void CodeGenerator::generateBlock(ASTNode* node) {
    for (ASTNode* child : node->children) {
        generateNode(child);
    }
}

void CodeGenerator::generateDeclarationPart(ASTNode* node) {
    for (ASTNode* child : node->children) {
        if (!child) continue;
        if (child->kind == ASTKind::ProcDecl || child->kind == ASTKind::FuncDecl) {
            generateSubprogramDecl(child);
        }
    }
}

void CodeGenerator::generateSubprogramDecl(ASTNode* node) {
    int ti = node->tabIndex;
    if (ti < 0 || ti >= sym_.tabSize()) return;

    const TabEntry& procEntry = sym_.tabAt(ti);

    // JMP melewati seluruh body; di-patch setelah body selesai di-emit
    int jmpSkip = buf_.emit(OpCode::JMP, 0, 0, "jmp skip body " + node->value);

    // catat alamat awal body untuk CAL nanti
    int bodyStart = buf_.currentAddress();
    subprogramAddr_[ti] = bodyStart;
    auto pending = pendingSubprogramCalls_.find(ti);
    if (pending != pendingSubprogramCalls_.end()) {
        for (int callAddress : pending->second) {
            buf_.patch(callAddress, bodyStart);
        }
        pendingSubprogramCalls_.erase(pending);
    }

    // hitung ukuran frame subprogram dari btab
    int bref = procEntry.ref;
    int frameSize = FRAME_HEADER;
    if (bref > 0 && bref < sym_.btabSize()) {
        frameSize = FRAME_HEADER + sym_.btabAt(bref).vsze + sym_.btabAt(bref).psze;
    }

    buf_.emit(OpCode::INT, currentLevel_ + 1, frameSize, "alokasi frame " + node->value);

    // TODO: Interpreter harus menyepakati layout return value function.
    // CAL/RET sudah di-emit, tetapi function result baru bisa divalidasi
    // end-to-end setelah activation record runtime selesai.
    std::string previousFunctionName = currentFunctionName_;
    if (node->kind == ASTKind::FuncDecl) {
        currentFunctionName_ = normalize(node->value);
    }

    // naikkan level saat generate body subprogram
    currentLevel_++;

    // generate body
    for (ASTNode* child : node->children) {
        if (!child) continue;
        if (child->kind == ASTKind::Block) {
            generateBlock(child);
        }
    }

    currentLevel_--;
    currentFunctionName_ = previousFunctionName;

    buf_.emit(OpCode::RET, 0, 0, "ret dari " + node->value);

    // patch JMP skip ke instruksi setelah RET
    buf_.patch(jmpSkip, buf_.currentAddress());
}

void CodeGenerator::generateStatement(ASTNode* node) {
    generateNode(node);
}


void CodeGenerator::generateAssign(ASTNode* node) {
    if (node->children.size() < 2) return;

    ASTNode* lhs = node->children[0];
    ASTNode* rhs = node->children[1];

    if (lhs->kind == ASTKind::ArrayAccess) {
        // Untuk array indirect store, urutan di stack: [RHS_value, abs_address]
        // Interpreter: pop address, pop value, simpan value ke address
        generateExpression(rhs);
        LValue lv = resolveLValue(lhs);
        buf_.emit(OpCode::STO, lv.level, lv.address,
                  "simpan ke " + (lhs->value.empty() ? "var" : lhs->value));
    } else {
        generateExpression(rhs);
        LValue lv = resolveLValue(lhs);
        buf_.emit(OpCode::STO, lv.level, lv.address,
                  "simpan ke " + (lhs->value.empty() ? "var" : lhs->value));
    }
}


void CodeGenerator::generateExpression(ASTNode* node) {
    if (!node) return;

    switch (node->kind) {
        case ASTKind::Literal:
            generateLiteral(node);
            break;
        case ASTKind::Var:
            generateVar(node);
            break;
        case ASTKind::BinOp:
            generateBinOp(node);
            break;
        case ASTKind::UnaryOp:
            generateUnaryOp(node);
            break;
        case ASTKind::ArrayAccess:
            generateArrayAccess(node);
            break;
        case ASTKind::FieldAccess:
            generateFieldAccess(node);
            break;
        case ASTKind::FunctionCall:
            generateFunctionCall(node);
            break;
        default:
            break;
    }
}


void CodeGenerator::generateLiteral(ASTNode* node) {
    const std::string& v = node->value;
    const SemanticType& t = node->inferredType;

    if (t.kind == TypeKind::Integer || t.kind == TypeKind::Subrange) {
        try {
            int ival = std::stoi(v);
            buf_.emit(OpCode::LIT, 0, ival, "literal int " + v);
        } catch (...) {
            buf_.emit(OpCode::LIT, 0, 0, "literal int (parse error)");
        }
    } else if (t.kind == TypeKind::Boolean) {
        std::string lv = normalize(v);
        int bval = (lv == "true") ? 1 : 0;
        buf_.emit(OpCode::LIT, 0, bval, "literal bool " + v);
    } else if (t.kind == TypeKind::Char) {
        int cval = 0;
        if (!v.empty()) {
            if (v.size() >= 3 && v.front() == '\'' && v.back() == '\'') {
                cval = static_cast<unsigned char>(v[1]);
            } else if (!v.empty()) {
                cval = static_cast<unsigned char>(v[0]);
            }
        }
        buf_.emit(OpCode::LIT, 0, cval, "literal char " + v);
    } else if (t.kind == TypeKind::Real) {
        try {
            double dval = std::stod(v);
            int ival = static_cast<int>(dval);
            buf_.emit(OpCode::LIT, 0, ival, "literal real (truncated) " + v);
        } catch (...) {
            buf_.emit(OpCode::LIT, 0, 0, "literal real (parse error)");
        }
    } else if (t.kind == TypeKind::String) {
        std::string s = v;
        if (s.size() >= 2 && s.front() == '\'' && s.back() == '\'') {
            s = s.substr(1, s.size() - 2);
        }
        buf_.emit(OpCode::LIT, 0, -1, "string:" + s);
    } else {
        try {
            int ival = std::stoi(v);
            buf_.emit(OpCode::LIT, 0, ival, "literal " + v);
        } catch (...) {
            buf_.emit(OpCode::LIT, 0, 0, "literal (fallback) " + v);
        }
    }
}

void CodeGenerator::generateVar(ASTNode* node) {
    int ti = node->tabIndex;
    if (ti < 0 || ti >= sym_.tabSize()) {
        buf_.emit(OpCode::LIT, 0, 0, "var (tabIndex invalid) " + node->value);
        return;
    }

    const TabEntry& e = sym_.tabAt(ti);

    if (e.obj == ObjectKind::Constant) {
        buf_.emit(OpCode::LIT, 0, e.adr, "const " + node->value);
        return;
    }

    int diff = currentLevel_ - e.lev;
    // adr dari SymbolTable mulai dari 0; tambahkan FRAME_HEADER agar slot 0-2
    // (static link, dynamic link, return address) tidak tertimpa.
    buf_.emit(OpCode::LOD, diff, e.adr + FRAME_HEADER, "load " + node->value);
}


void CodeGenerator::generateBinOp(ASTNode* node) {
    if (node->children.size() < 2) return;

    ASTNode* left  = node->children[0];
    ASTNode* right = node->children[1];

    const std::string& op = normalize(node->value);
    generateExpression(left);
    generateExpression(right);

    int oprCode = oprForBinOp(op);
    buf_.emit(OpCode::OPR, 0, oprCode, "op " + node->value);
}


void CodeGenerator::generateUnaryOp(ASTNode* node) {
    if (node->children.empty()) return;

    generateExpression(node->children[0]);

    const std::string& op = normalize(node->value);
    int oprCode = oprForUnaryOp(op);
    buf_.emit(OpCode::OPR, 0, oprCode, "unary " + node->value);
}


void CodeGenerator::generateArrayAccess(ASTNode* node) {
    if (node->children.size() < 2) return;

    ASTNode* arrayVarNode = node->children[0];
    ASTNode* indexExpr    = node->children[1];

    int ti = arrayVarNode->tabIndex;
    if (ti < 0 || ti >= sym_.tabSize()) {
        buf_.emit(OpCode::LIT, 0, 0, "array (tabIndex invalid) " + node->value);
        return;
    }

    const TabEntry& arrEntry = sym_.tabAt(ti);
    const SemanticType& arrType = arrEntry.type;

    if (arrType.kind != TypeKind::Array) {
        int diff = currentLevel_ - arrEntry.lev;
        buf_.emit(OpCode::LOD, diff, arrEntry.adr + FRAME_HEADER, "load (non-array) " + node->value);
        return;
    }

    int aref = arrType.ref;
    if (aref <= 0 || aref >= sym_.atabSize()) {
        buf_.emit(OpCode::LIT, 0, 0, "array (atab ref invalid)");
        return;
    }

    const ATabEntry& ae = sym_.atabAt(aref);
    int low  = ae.low;
    int elsz = (ae.elsz > 0) ? ae.elsz : 1;

    int diff = currentLevel_ - arrEntry.lev;
    // base addr dengan FRAME_HEADER offset
    buf_.emit(OpCode::LIT, 0, arrEntry.adr + FRAME_HEADER, "array base addr " + node->value);

    generateExpression(indexExpr);
    if (low != 0) {
        buf_.emit(OpCode::LIT, 0, low, "array low bound");
        buf_.emit(OpCode::OPR, 0, OPR::SUB, "index - low");
    }
    if (elsz != 1) {
        buf_.emit(OpCode::LIT, 0, elsz, "element size");
        buf_.emit(OpCode::OPR, 0, OPR::MUL, "offset = (idx-low)*elsz");
    }

    buf_.emit(OpCode::OPR, 0, OPR::ADD, "array abs addr");

    buf_.emit(OpCode::LOD, diff, -1, "load array[idx]");
}

void CodeGenerator::generateFieldAccess(ASTNode* node) {
    if (node->children.empty()) return;

    ASTNode* baseNode = node->children[0];

    int baseTi = baseNode->tabIndex;
    if (baseTi < 0 || baseTi >= sym_.tabSize()) {
        buf_.emit(OpCode::LIT, 0, 0, "field access (base tabIndex invalid)");
        return;
    }

    const TabEntry& baseEntry = sym_.tabAt(baseTi);

    int fieldTi = node->tabIndex;
    if (fieldTi < 0 || fieldTi >= sym_.tabSize()) {
        buf_.emit(OpCode::LIT, 0, 0, "field access (field tabIndex invalid) " + node->value);
        return;
    }

    const TabEntry& fieldEntry = sym_.tabAt(fieldTi);

    // base addr + field offset, kemudian tambahkan FRAME_HEADER untuk slot header
    int absAddr = baseEntry.adr + fieldEntry.adr + FRAME_HEADER;
    int diff = currentLevel_ - baseEntry.lev;
    buf_.emit(OpCode::LOD, diff, absAddr, "load " + baseNode->value + "." + node->value);
}

CodeGenerator::LValue CodeGenerator::resolveLValue(ASTNode* node) {
    if (!node) return {0, 0};

    switch (node->kind) {
        case ASTKind::Var: {
            int ti = node->tabIndex;
            if (ti < 0 || ti >= sym_.tabSize()) return {0, 0};
            const TabEntry& e = sym_.tabAt(ti);
            int diff = currentLevel_ - e.lev;
            // tambahkan FRAME_HEADER ke alamat variabel
            return {diff, e.adr + FRAME_HEADER};
        }
        case ASTKind::ArrayAccess:
            return resolveLValueArrayAccess(node);
        case ASTKind::FieldAccess:
            return resolveLValueFieldAccess(node);
        default:
            return {0, 0};
    }
}

CodeGenerator::LValue CodeGenerator::resolveLValueArrayAccess(ASTNode* node) {
    // Struktur node: children[0]=Var(array), children[1]=ekspresi indeks
    if (node->children.size() < 2) return {0, 0};

    ASTNode* arrayVarNode = node->children[0];
    ASTNode* indexExpr    = node->children[1];

    int ti = arrayVarNode->tabIndex;
    if (ti < 0 || ti >= sym_.tabSize()) return {0, -1};

    const TabEntry& arrEntry = sym_.tabAt(ti);
    const SemanticType& arrType = arrEntry.type;

    if (arrType.kind != TypeKind::Array) {
        return {currentLevel_ - arrEntry.lev, arrEntry.adr + FRAME_HEADER};
    }

    int aref = arrType.ref;
    if (aref <= 0 || aref >= sym_.atabSize()) return {0, -1};

    const ATabEntry& ae = sym_.atabAt(aref);
    int low  = ae.low;
    int elsz = (ae.elsz > 0) ? ae.elsz : 1;

    int diff = currentLevel_ - arrEntry.lev;

    buf_.emit(OpCode::LIT, 0, arrEntry.adr + FRAME_HEADER, "array base addr (LValue)");
    generateExpression(indexExpr);
    if (low != 0) {
        buf_.emit(OpCode::LIT, 0, low, "array low (LValue)");
        buf_.emit(OpCode::OPR, 0, OPR::SUB, "idx-low (LValue)");
    }
    if (elsz != 1) {
        buf_.emit(OpCode::LIT, 0, elsz, "elsz (LValue)");
        buf_.emit(OpCode::OPR, 0, OPR::MUL, "offset (LValue)");
    }
    buf_.emit(OpCode::OPR, 0, OPR::ADD, "abs addr (LValue)");

    return {diff, -1, true};
}

CodeGenerator::LValue CodeGenerator::resolveLValueFieldAccess(ASTNode* node) {
    if (node->children.empty()) return {0, 0};

    ASTNode* baseNode = node->children[0];
    int baseTi = baseNode->tabIndex;
    if (baseTi < 0 || baseTi >= sym_.tabSize()) return {0, 0};

    const TabEntry& baseEntry = sym_.tabAt(baseTi);

    int fieldTi = node->tabIndex;
    if (fieldTi < 0 || fieldTi >= sym_.tabSize()) return {0, 0};

    const TabEntry& fieldEntry = sym_.tabAt(fieldTi);

    // base addr + field offset + FRAME_HEADER
    int absAddr = baseEntry.adr + fieldEntry.adr + FRAME_HEADER;
    int diff = currentLevel_ - baseEntry.lev;
    return {diff, absAddr};
}

void CodeGenerator::generateWrite(ASTNode* node, bool newline) {
    size_t n = node->children.size();
    for (size_t i = 0; i < n; ++i) {
        generateExpression(node->children[i]);
        bool isLast = (i == n - 1);
        if (isLast && newline) {
            buf_.emit(OpCode::OPR, 0, OPR::WRTLN, "writeln arg " + std::to_string(i));
        } else {
            buf_.emit(OpCode::OPR, 0, OPR::WRT, "write arg " + std::to_string(i));
        }
    }
    if (n == 0 && newline) {
        buf_.emit(OpCode::LIT, 0, -1, "string:"); 
        buf_.emit(OpCode::OPR, 0, OPR::WRTLN, "writeln (no args)");
    }
}

void CodeGenerator::generateRead(ASTNode* node, bool newline) {
    for (ASTNode* child : node->children) {
        if (child && child->kind == ASTKind::ArrayAccess && !child->children.empty()) {
            ASTNode* base = child->children[0];
            int level = 0;
            if (base && base->tabIndex >= 0 && base->tabIndex < sym_.tabSize()) {
                level = currentLevel_ - sym_.tabAt(base->tabIndex).lev;
            }
            buf_.emit(OpCode::LIT, 0, level, "read target lexical level");
            resolveLValue(child);
        } else {
            LValue lv = resolveLValue(child);
            buf_.emit(OpCode::LIT, 0, lv.level, "read target lexical level");
            if (!lv.isStack) {
                buf_.emit(OpCode::LIT, 0, lv.address, "read target address");
            }
        }
        // TODO: OPR READ/READLN harus pop address lalu level,
        // membaca input runtime, dan store ke alamat target. Untuk array
        // indirect, resolveLValue sudah meninggalkan alamat target di stack.
        buf_.emit(OpCode::OPR, 0, newline ? OPR::READLN : OPR::READ,
                  (newline ? "readln " : "read ") + child->value);
    }
}

void CodeGenerator::emitSubprogramCall(ASTNode* node, const TabEntry& entry) {
    int ti = node->tabIndex;
    int diff = currentLevel_ - entry.lev;
    int bodyAddr = 0;

    auto it = subprogramAddr_.find(ti);
    if (it != subprogramAddr_.end()) {
        bodyAddr = it->second;
    }

    for (ASTNode* child : node->children) {
        generateExpression(child);
    }

    int callAddress = buf_.emit(OpCode::CAL, diff, bodyAddr, "call " + node->value);
    if (it == subprogramAddr_.end()) {
        pendingSubprogramCalls_[ti].push_back(callAddress);
    }
}


void CodeGenerator::generateProcedureCall(ASTNode* node) {
    std::string name = normalize(node->value);

    if (name == "writeln") {
        generateWrite(node, true);
        return;
    }
    if (name == "write") {
        generateWrite(node, false);
        return;
    }
    if (name == "readln" || name == "read") {
        generateRead(node, name == "readln");
        return;
    }

    // User-defined procedure: emit argumen lalu CAL. Parameter binding dan
    // activation record runtime adalah kontrak bersama dengan Interpreter.
    int ti = node->tabIndex;
    if (ti < 0 || ti >= sym_.tabSize()) return;
    const TabEntry& procEntry = sym_.tabAt(ti);
    emitSubprogramCall(node, procEntry);
}

void CodeGenerator::generateFunctionCall(ASTNode* node) {
    int ti = node->tabIndex;
    if (ti < 0 || ti >= sym_.tabSize()) return;
    const TabEntry& funcEntry = sym_.tabAt(ti);
    emitSubprogramCall(node, funcEntry);
    // Baca return value dari slot nama fungsi di frame caller
    int diff = currentLevel_ - funcEntry.lev;
    buf_.emit(OpCode::LOD, diff, funcEntry.adr + FRAME_HEADER,
              "load return value " + node->value);
}

void CodeGenerator::generateIf(ASTNode* node) {
    // Struktur children: [0]=kondisi, [1]=then-body, [2]=else-body (opsional)
    if (node->children.empty()) return;

    bool hasElse = (node->children.size() >= 3);

    // emit kondisi
    generateExpression(node->children[0]);

    // JPC ke else (atau ke end jika tidak ada else); alamat di-patch nanti
    int jpcAddr = buf_.emit(OpCode::JPC, 0, 0, "jpc ke else/end");

    // then body
    generateNode(node->children[1]);

    if (hasElse) {
        // JMP melewati else body; alamat di-patch nanti
        int jmpAddr = buf_.emit(OpCode::JMP, 0, 0, "jmp ke end if");

        // patch JPC ke awal else
        buf_.patch(jpcAddr, buf_.currentAddress());

        // else body
        generateNode(node->children[2]);

        // patch JMP ke instruksi sesudah else
        buf_.patch(jmpAddr, buf_.currentAddress());
    } else {
        // patch JPC ke instruksi sesudah then
        buf_.patch(jpcAddr, buf_.currentAddress());
    }
}

void CodeGenerator::generateWhile(ASTNode* node) {
    // Struktur children: [0]=kondisi, [1]=body
    if (node->children.size() < 2) return;

    // simpan alamat awal evaluasi kondisi
    int loopStart = buf_.currentAddress();

    // emit kondisi
    generateExpression(node->children[0]);

    // JPC ke sesudah loop jika kondisi false; di-patch nanti
    int jpcAddr = buf_.emit(OpCode::JPC, 0, 0, "jpc ke akhir while");

    // body
    generateNode(node->children[1]);

    // lompat kembali ke awal kondisi
    buf_.emit(OpCode::JMP, 0, loopStart, "jmp ke awal while");

    // patch JPC ke instruksi setelah JMP
    buf_.patch(jpcAddr, buf_.currentAddress());
}

void CodeGenerator::generateRepeat(ASTNode* node) {
    // Struktur children: [0]=body, [1]=kondisi until
    if (node->children.size() < 2) return;

    // simpan alamat awal body
    int loopStart = buf_.currentAddress();

    // body
    generateNode(node->children[0]);

    // emit kondisi until
    generateExpression(node->children[1]);
    buf_.emit(OpCode::JPC, 0, loopStart, "jpc ke awal repeat jika belum selesai");
}

void CodeGenerator::generateFor(ASTNode* node) {
    if (node->children.size() < 3) return;

    int ti = node->tabIndex;
    if (ti < 0 || ti >= sym_.tabSize()) return;
    const TabEntry& ctrlEntry = sym_.tabAt(ti);
    int diff   = currentLevel_ - ctrlEntry.lev;
    int ctrlAdr = ctrlEntry.adr + FRAME_HEADER;

    bool isDownto = (node->attribute == "downto");

    // assign nilai awal ke variabel kontrol
    generateExpression(node->children[0]);
    buf_.emit(OpCode::STO, diff, ctrlAdr, "for: init " + node->value);

    // awal pengecekan kondisi
    int loopStart = buf_.currentAddress();

    // load variabel kontrol dan nilai akhir, lalu bandingkan
    buf_.emit(OpCode::LOD, diff, ctrlAdr, "for: load " + node->value);
    generateExpression(node->children[1]);

    if (isDownto) {
        // kondisi lanjut: ctrlVar >= limitExpr (GEQ = 10)
        buf_.emit(OpCode::OPR, 0, OPR::GEQ, "for downto: cek >= limit");
    } else {
        // kondisi lanjut: ctrlVar <= limitExpr (LEQ = 12)
        buf_.emit(OpCode::OPR, 0, OPR::LEQ, "for to: cek <= limit");
    }

    // JPC keluar jika kondisi false
    int jpcAddr = buf_.emit(OpCode::JPC, 0, 0, "jpc ke akhir for");

    // body
    generateNode(node->children[2]);

    // increment atau decrement variabel kontrol
    buf_.emit(OpCode::LOD, diff, ctrlAdr, "for: load " + node->value + " for step");
    buf_.emit(OpCode::LIT, 0, 1, "for: step 1");
    if (isDownto) {
        buf_.emit(OpCode::OPR, 0, OPR::SUB, "for downto: decrement");
    } else {
        buf_.emit(OpCode::OPR, 0, OPR::ADD, "for to: increment");
    }
    buf_.emit(OpCode::STO, diff, ctrlAdr, "for: update " + node->value);

    // lompat ke awal pengecekan
    buf_.emit(OpCode::JMP, 0, loopStart, "jmp ke awal for");

    // patch JPC ke sesudah loop
    buf_.patch(jpcAddr, buf_.currentAddress());
}

void CodeGenerator::generateCase(ASTNode* node) {
    if (!node || node->children.empty()) return;

    ASTNode* selector = node->children[0];
    std::vector<int> endJumps;

    for (size_t i = 1; i < node->children.size(); ++i) {
        generateCaseBranch(selector, node->children[i], endJumps);
    }

    int endAddress = buf_.currentAddress();
    for (int jumpAddress : endJumps) {
        buf_.patch(jumpAddress, endAddress);
    }
}

void CodeGenerator::generateCaseBranch(ASTNode* selector, ASTNode* branch, std::vector<int>& endJumps) {
    if (!selector || !branch || branch->kind != ASTKind::Case) return;

    std::vector<ASTNode*> labels;
    std::vector<ASTNode*> statements;
    std::vector<ASTNode*> nextBranches;

    bool seenStatement = false;
    for (ASTNode* child : branch->children) {
        if (!child) continue;
        if (!seenStatement &&
            (child->kind == ASTKind::Literal || child->kind == ASTKind::Var)) {
            labels.push_back(child);
        } else if (child->kind == ASTKind::Case) {
            nextBranches.push_back(child);
        } else {
            seenStatement = true;
            statements.push_back(child);
        }
    }

    for (ASTNode* label : labels) {
        generateExpression(selector);
        generateExpression(label);
        buf_.emit(OpCode::OPR, 0, OPR::EQL, "case: selector = label");

        int missJump = buf_.emit(OpCode::JPC, 0, 0, "case: next label");
        for (ASTNode* statement : statements) {
            generateNode(statement);
        }
        endJumps.push_back(buf_.emit(OpCode::JMP, 0, 0, "case: end"));
        buf_.patch(missJump, buf_.currentAddress());
    }

    for (ASTNode* nextBranch : nextBranches) {
        generateCaseBranch(selector, nextBranch, endJumps);
    }
}

int CodeGenerator::typeSize(const SemanticType& t) const {
    switch (t.kind) {
        case TypeKind::Integer:
        case TypeKind::Boolean:
        case TypeKind::Char:
        case TypeKind::Subrange:
        case TypeKind::Enumerated:
            return 1;
        case TypeKind::Real:
            return 1;  
        case TypeKind::String:
            return 1;  
        case TypeKind::Array: {
            if (t.ref > 0 && t.ref < sym_.atabSize()) {
                return sym_.atabAt(t.ref).size;
            }
            return 1;
        }
        case TypeKind::Record: {
            if (t.ref > 0 && t.ref < sym_.btabSize()) {
                return sym_.btabAt(t.ref).vsze;
            }
            return 1;
        }
        default:
            return 1;
    }
}

int CodeGenerator::oprForBinOp(const std::string& op) const {
    // op sudah dinormalize (lowercase)
    if (op == "+")   return OPR::ADD;
    if (op == "-")   return OPR::SUB;
    if (op == "*")   return OPR::MUL;
    if (op == "/")   return OPR::DIV;
    if (op == "div") return OPR::DIV;
    if (op == "mod") return OPR::MOD;
    if (op == "=")   return OPR::EQL;
    if (op == "<>")  return OPR::NEQ;
    if (op == "<")   return OPR::LSS;
    if (op == ">=")  return OPR::GEQ;
    if (op == ">")   return OPR::GTR;
    if (op == "<=")  return OPR::LEQ;
    if (op == "and") return OPR::AND;
    if (op == "or")  return OPR::OR;
    return OPR::ADD;
}


int CodeGenerator::oprForUnaryOp(const std::string& op) const {
    if (op == "-")   return OPR::NEG;
    if (op == "not") return OPR::NOT;
    if (op == "+")   return OPR::ADD; 
    return OPR::NEG;
}

std::string CodeGenerator::normalize(const std::string& s) {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return r;
}
