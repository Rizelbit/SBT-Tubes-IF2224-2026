#pragma once
#include "AST.hpp"
#include "TypeSystem.hpp"
#include <ostream>
#include <string>
#include <vector>


enum class ObjectKind {
    Program,
    Constant,
    Type,
    Variable,
    Parameter,  
    VarParam,   
    Field,      
    Procedure,
    Function
};

std::string objectKindToString(ObjectKind ok);


struct TabEntry {
    std::string  identifier;                       
    int link = -1;                  
    ObjectKind obj = ObjectKind::Variable;
    SemanticType type;                              
    int ref = 0;                   
    int nrm = 1;                   
    int lev = 0;                   
    int adr = 0;                   
    bool initialized = false;               
};

// btab
struct BTabEntry {
    int last = 0;  
    int lpar = 0;  
    int psze = 0;  
    int vsze = 0;  
};

// atab
struct ATabEntry {
    SemanticType xtyp; 
    SemanticType etyp;       
    int eref = 0; 
    int low  = 0;  
    int high = 0; 
    int elsz = 0;  
    int size = 0;  
};


int getTypeSize(const SemanticType& t);

class SymbolTable {
public:
    SymbolTable();
    void initPredefined();
    int  enterBlock();              
    void leaveBlock();             
    int  currentLevel() const;
    int  currentBlock() const;


    int declare(const std::string& name, ObjectKind obj, const SemanticType& type, int nrm = 1, int adrOverride = -1);
    int lookup(const std::string& name) const;  
    int lookupCurrentScope(const std::string& name) const; 
    int lookupInBlock(const std::string& name, int blockIdx) const;


    TabEntry& tabAt(int i) { return tab_[i]; }
    const TabEntry& tabAt(int i) const { return tab_[i]; }
    BTabEntry& btabAt(int i) { return btab_[i]; }
    const BTabEntry& btabAt(int i) const { return btab_[i]; }
    ATabEntry& atabAt(int i) { return atab_[i]; }
    const ATabEntry& atabAt(int i) const { return atab_[i]; }

    int tabSize() const { return static_cast<int>(tab_.size());}
    int btabSize() const { return static_cast<int>(btab_.size());}
    int atabSize() const { return static_cast<int>(atab_.size()); }

    int addArrayType(const ATabEntry& entry); 
    int addBlock(const BTabEntry& entry);    

    SemanticType resolveTypeName(const std::string& name) const;
    void printTables(std::ostream& out) const;

private:
    std::vector<TabEntry>  tab_;
    std::vector<BTabEntry> btab_;
    std::vector<ATabEntry> atab_;

    std::vector<int> scopeStart_;
    std::vector<int> addrCounter_;
    std::vector<int> btabStack_;

    int currentLevel_ = 0;

    static std::string normalize(const std::string& s);
};