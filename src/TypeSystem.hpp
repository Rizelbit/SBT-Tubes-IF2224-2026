#pragma once
#include "AST.hpp"
#include <string>

inline SemanticType makeType(TypeKind k, int ref = 0) {
    SemanticType t;
    t.kind = k;
    t.ref  = ref;
    return t;
}

inline SemanticType makeIntType(){ 
    return makeType(TypeKind::Integer); 
}

inline SemanticType makeRealType(){ 
    return makeType(TypeKind::Real);    
}

inline SemanticType makeCharType(){ 
    return makeType(TypeKind::Char);    
}

inline SemanticType makeBoolType(){ 
    return makeType(TypeKind::Boolean); 
}

inline SemanticType makeStringType(){ 
    return makeType(TypeKind::String);  
}

inline SemanticType makeVoidType(){ 
    return makeType(TypeKind::Void);    
}

inline SemanticType makeUnknownType(){ 
    return makeType(TypeKind::Unknown); 
}



// integer/real
bool isNumericType(const SemanticType& t);

// ordinal (integer, char, boolean, enumerated, subrange)
bool isOrdinalType(const SemanticType& t);

bool isCompatible(const SemanticType& left, const SemanticType& right);

bool isAssignmentCompatible(const SemanticType& target, const SemanticType& value);

// untuk tipe hasil operasi binary
SemanticType inferBinary(const std::string& op,const SemanticType& left,const SemanticType& right);

// untuk tipe hasil operasi unary
SemanticType inferUnary(const std::string& op, const SemanticType& operand);

std::string typeKindToString(TypeKind k);
std::string typeToString(const SemanticType& t);