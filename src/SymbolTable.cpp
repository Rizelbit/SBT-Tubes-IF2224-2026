#include "SymbolTable.hpp"
#include <algorithm>
#include <cctype>
#include <iomanip>

std::string objectKindToString(ObjectKind ok) {
    switch (ok) {
        case ObjectKind::Program: return "Program";
        case ObjectKind::Constant: return "Constant";
        case ObjectKind::Type: return "Type";
        case ObjectKind::Variable: return "Variable";
        case ObjectKind::Parameter: return "Parameter";
        case ObjectKind::VarParam: return "VarParam";
        case ObjectKind::Field: return "Field";
        case ObjectKind::Procedure: return "Procedure";
        case ObjectKind::Function: return "Function";
    }
    return "Unknown";
}

int getTypeSize(const SemanticType& t) {
    switch (t.kind) {
        case TypeKind::Integer:
        case TypeKind::Char:
        case TypeKind::Boolean:
        case TypeKind::Subrange:
        case TypeKind::Enumerated: return 1;
        case TypeKind::Real: return 2;
        case TypeKind::String: return 1;    //
        case TypeKind::Array:
        case TypeKind::Record: return 0;    
        default:
            return 0;
    }
}

std::string SymbolTable::normalize(const std::string& s) {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(), [](unsigned char c){ return std::tolower(c); });
    return r;
}

SymbolTable::SymbolTable() {
    btab_.push_back({0, 0, 0, 0});
    atab_.push_back({});
    for (int i = 0; i < 33; ++i) {
        TabEntry dummy;
        dummy.identifier = "__reserved__";
        dummy.lev = -1; 
        tab_.push_back(dummy);
    }

    scopeStart_.push_back(33);
    addrCounter_.push_back(0);
    btabStack_.push_back(0);   
    currentLevel_ = 0;
}

void SymbolTable::initPredefined() {
    auto addType = [&](const std::string& name, TypeKind k) {
        TabEntry e;
        e.identifier = name;
        e.obj = ObjectKind::Type;
        e.type = makeType(k);
        e.lev = 0;
        e.link = btab_[btabStack_.back()].last; 
        int idx = static_cast<int>(tab_.size());
        tab_.push_back(e);
        btab_[btabStack_.back()].last = idx;     
    };
    addType("Integer", TypeKind::Integer);
    addType("Real", TypeKind::Real);
    addType("Char", TypeKind::Char);
    addType("Boolean",TypeKind::Boolean);
    addType("String", TypeKind::String);

    auto addBoolConst = [&](const std::string& name, int ordinal) {
        TabEntry e;
        e.identifier   = name;
        e.obj = ObjectKind::Constant;
        e.type = makeBoolType();
        e.adr = ordinal;
        e.lev = 0;
        e.initialized  = true;                       
        e.link = btab_[btabStack_.back()].last;
        int idx = static_cast<int>(tab_.size());
        tab_.push_back(e);
        btab_[btabStack_.back()].last = idx;
    };
    addBoolConst("True",  1);
    addBoolConst("False", 0);

    auto addProc = [&](const std::string& name) {
        BTabEntry b;
        b.last = static_cast<int>(tab_.size());
        b.lpar = 0;
        b.psze = 0;
        b.vsze = 0;
        int bIdx = static_cast<int>(btab_.size());
        btab_.push_back(b);

        TabEntry e;
        e.identifier = name;
        e.obj = ObjectKind::Procedure;
        e.type = makeVoidType();
        e.ref = bIdx;   
        e.adr = 0;      
        e.lev = 0;
        e.link = btab_[btabStack_.back()].last;
        int idx = static_cast<int>(tab_.size());
        tab_.push_back(e);
        btab_[btabStack_.back()].last = idx;
    };
    addProc("writeln");
    addProc("readln");
    addProc("write");
    addProc("read");
}

int SymbolTable::enterBlock() {
    currentLevel_++;
    scopeStart_.push_back(static_cast<int>(tab_.size()));
    addrCounter_.push_back(0);
    BTabEntry b;
    b.last = static_cast<int>(tab_.size()); 
    b.lpar = 0;
    b.psze = 0;
    b.vsze = 0;
    int bIdx = static_cast<int>(btab_.size());
    btab_.push_back(b);
    btabStack_.push_back(bIdx);  
    return bIdx;
}


void SymbolTable::leaveBlock() {
    if (currentLevel_ <= 0) return;
    int bIdx = btabStack_.back();
    if (static_cast<int>(tab_.size()) > scopeStart_.back()){
        btab_[bIdx].last = static_cast<int>(tab_.size()) - 1;
    }
    btab_[bIdx].vsze = addrCounter_.back();
    scopeStart_.pop_back();
    addrCounter_.pop_back();
    btabStack_.pop_back();
    currentLevel_--;
}

int SymbolTable::currentLevel() const {return currentLevel_; }

int SymbolTable::currentBlock() const {
    if (btabStack_.empty()){
        return 0;
    } 
    else {
        return btabStack_.back();
    }
}

int SymbolTable::declare(const std::string& name, ObjectKind obj, const SemanticType& type, int nrm, int adrOverride) {
    if (lookupCurrentScope(name) != -1)
        return -1;
    TabEntry e;
    e.identifier = name;
    e.obj = obj;
    e.type = type;
    e.nrm = nrm;
    e.lev = currentLevel_;

    int bIdx = btabStack_.back();
    e.link   = btab_[bIdx].last;

    if (adrOverride >= 0) {
        e.adr = adrOverride;
    } else {
        bool needsOffset = (obj == ObjectKind::Variable || obj == ObjectKind::Parameter || obj == ObjectKind::VarParam  || obj == ObjectKind::Field);
        if (needsOffset) {
            e.adr = addrCounter_.back();
            int sz = getTypeSize(type);
            addrCounter_.back() += (sz > 0 ? sz : 1);
        } else {
            e.adr = 0;
        }
    }

    int idx = static_cast<int>(tab_.size());
    tab_.push_back(e);
    btab_[bIdx].last = idx;

    return idx;
}

int SymbolTable::lookup(const std::string& name) const {
    std::string key = normalize(name);
    for (int i = static_cast<int>(tab_.size()) - 1; i >= 0; --i) {
        if (tab_[i].lev == -1) continue;    
        if (tab_[i].lev > currentLevel_) continue;    
        if (normalize(tab_[i].identifier) == key)
            return i;
    }
    return -1;
}

int SymbolTable::lookupInBlock(const std::string& name, int blockIdx) const {
    if (blockIdx < 0 || blockIdx >= static_cast<int>(btab_.size())) return -1;
    std::string key = normalize(name);
    int current = btab_[blockIdx].last;
    while (current > 0 && current < static_cast<int>(tab_.size())) {
        if (normalize(tab_[current].identifier) == key) {
            return current;
        }
        current = tab_[current].link;
    }
    return -1;
}

int SymbolTable::lookupCurrentScope(const std::string& name) const {
    std::string key   = normalize(name);
    int         start = scopeStart_.back();
    for (int i = static_cast<int>(tab_.size()) - 1; i >= start; --i) {
        if (tab_[i].lev == -1) continue;
        if (normalize(tab_[i].identifier) == key)
            return i;
    }
    return -1;
}

int SymbolTable::addArrayType(const ATabEntry& entry) {
    int idx = static_cast<int>(atab_.size());
    atab_.push_back(entry);
    return idx;
}

int SymbolTable::addBlock(const BTabEntry& entry) {
    int idx = static_cast<int>(btab_.size());
    btab_.push_back(entry);
    return idx;
}

SemanticType SymbolTable::resolveTypeName(const std::string& name) const {
    int idx = lookup(name);
    if (idx == -1) return makeUnknownType();
    const TabEntry& e = tab_[idx];
    if (e.obj != ObjectKind::Type) return makeUnknownType();
    return e.type;
}

void SymbolTable::printTables(std::ostream& out) const {
    out << "Symbol Table - tab:\n";
    out << std::left
        << std::setw(5)  << "idx"
        << std::setw(16) << "identifier"
        << std::setw(12) << "obj"
        << std::setw(12) << "type"
        << std::setw(6)  << "ref"
        << std::setw(6)  << "nrm"
        << std::setw(6)  << "lev"
        << std::setw(6)  << "adr"
        << std::setw(6)  << "link"
        << "init"
        << "\n";
    out << std::string(81, '-') << "\n";

    for (int i = 0; i < static_cast<int>(tab_.size()); ++i) {
        const TabEntry& e = tab_[i];
        if (e.lev == -1) continue;
        out << std::left
            << std::setw(5)  << i
            << std::setw(16) << e.identifier
            << std::setw(12) << objectKindToString(e.obj)
            << std::setw(12) << typeToString(e.type)
            << std::setw(6)  << e.ref
            << std::setw(6)  << e.nrm
            << std::setw(6)  << e.lev
            << std::setw(6)  << e.adr
            << std::setw(6)  << e.link
            << (e.initialized ? "yes" : "no")
            << "\n";
    }

    out << "\nSymbol Table - btab:\n";
    out << std::left
        << std::setw(5) << "idx"
        << std::setw(8) << "last"
        << std::setw(8) << "lpar"
        << std::setw(8) << "psze"
        << "vsze"
        << "\n";
    out << std::string(33, '-') << "\n";

    for (int i = 0; i < static_cast<int>(btab_.size()); ++i) {
        const BTabEntry& b = btab_[i];
        out << std::left
            << std::setw(5) << i
            << std::setw(8) << b.last
            << std::setw(8) << b.lpar
            << std::setw(8) << b.psze
            << b.vsze
            << "\n";
    }

    out << "\nSymbol Table - atab:\n";
    out << std::left
        << std::setw(5)  << "idx"
        << std::setw(12) << "xtyp"
        << std::setw(12) << "etyp"
        << std::setw(6)  << "eref"
        << std::setw(6)  << "low"
        << std::setw(6)  << "high"
        << std::setw(6)  << "elsz"
        << "size"
        << "\n";
    out << std::string(59, '-') << "\n";

    for (int i = 1; i < static_cast<int>(atab_.size()); ++i) { 
        const ATabEntry& a = atab_[i];
        out << std::left
            << std::setw(5)  << i
            << std::setw(12) << typeToString(a.xtyp)
            << std::setw(12) << typeToString(a.etyp)
            << std::setw(6)  << a.eref
            << std::setw(6)  << a.low
            << std::setw(6)  << a.high
            << std::setw(6)  << a.elsz
            << a.size
            << "\n";
    }
}