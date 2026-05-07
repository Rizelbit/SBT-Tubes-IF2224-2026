#include "Parser.hpp"

ParseNode* Parser::parseAssignmentStatement(){
    ParseNode* node = new ParseNode("<assignment-statement>");
    // node->addChild(match(TokenType::IDENT));
    node->addChild(parseVariable());
    node->addChild(match(TokenType::BECOMES)); // :=
    node->addChild(parseExpression());
    return node;
}

ParseNode* Parser::parseProcedureFunctionCall(){
    ParseNode* node = new ParseNode("<procedure/function-call>");
    
    // harus diawali dengan identifier (nama fungsi atau prosedur)
    node->addChild(match(TokenType::IDENT));
    
    if (peekToken().type == TokenType::LPARENT) {
        node->addChild(match(TokenType::LPARENT));
        node->addChild(parseParameterList()); // Parse isi parameter
        node->addChild(match(TokenType::RPARENT));
    }
    
    return node;
}
ParseNode* Parser::parseParameterList(){
    ParseNode* node = new ParseNode("<parameter-list>");
    
    // Parsing ekspresi / argumen pertama
    node->addChild(parseExpression());
    
    // Selama masih ada argumen lain yang dipisahkan koma
    while (peekToken().type == TokenType::COMMA) {
        node->addChild(match(TokenType::COMMA));
        node->addChild(parseExpression());
    }
    
    return node;
}

ParseNode* Parser::parseExpression(){
    ParseNode* node = new ParseNode("<expression>");
    
    // simple term di awal
    node->addChild(parseSimpleExpression());

    // jika ada operator relasi, misal =,<,>,<=,>=
    TokenType cur = peekToken().type;
    if (cur == TokenType::EQL || cur == TokenType::NEQ || cur == TokenType::LSS || 
        cur == TokenType::LEQ || cur == TokenType::GTR || cur == TokenType::GEQ) {
        
        node->addChild(parseRelationalOperator());
        node->addChild(parseSimpleExpression());
    }

    return node;
}

ParseNode* Parser::parseSimpleExpression(){
    ParseNode* node = new ParseNode("<simple-expression>");
    
    // Cek opsional unary sign di awal ekspresi (misal: -5 atau +10)
    TokenType cur = peekToken().type;
    if (cur == TokenType::PLUS || cur == TokenType::MINUS) {
        node->addChild(match(cur));
    }
    
    //  term
    node->addChild(parseTerm());
    
    // Selama token berikutnya adalah additive operator (+, -, OR)
    cur = peekToken().type;
    while (cur == TokenType::PLUS || cur == TokenType::MINUS || cur == TokenType::ORSY) {
        node->addChild(parseAdditiveOperator());
        node->addChild(parseTerm());
        cur = peekToken().type;
    }
    
    return node;
}
static bool isMultiplicativeOperator(TokenType type) {
    return type == TokenType::TIMES || 
           type == TokenType::RDIV  ||
           type == TokenType::IDIV  || 
           type == TokenType::IMOD  || 
           type == TokenType::ANDSY;
}

ParseNode* Parser::parseTerm(){
    ParseNode* node = new ParseNode("<term>");
    
    // factor
    node->addChild(parseFactor());

    // (multiplicative-operator + factor)*
    while (isMultiplicativeOperator(peekToken().type)) {
        node->addChild(parseMultiplicativeOperator());
        node->addChild(parseFactor());
    }

    return node;
}

ParseNode* Parser::parseFactor(){
    ParseNode* node = new ParseNode("<factor>");
    TokenType cur = peekToken().type;
    
    if (cur == TokenType::IDENT) {
        if (peekToken(1).type == TokenType::LPARENT) {
            node->addChild(parseProcedureFunctionCall());
        } else {
            node->addChild(parseVariable());
        }
    } 
    // Literal konstanta
    else if (cur == TokenType::INTCON || cur == TokenType::REALCON || 
             cur == TokenType::STRING || cur == TokenType::CHARCON) {
        node->addChild(match(cur));
    } 
    // Ekspresi di dalam kurung ( )
    else if (cur == TokenType::LPARENT) {
        node->addChild(match(TokenType::LPARENT));
        node->addChild(parseExpression());
        node->addChild(match(TokenType::RPARENT));
    } 
    // Format pertidaksamaan boolean (NOT)
    else if (cur == TokenType::NOTSY) {
        node->addChild(match(TokenType::NOTSY));
        node->addChild(parseFactor()); // Rekursif ke factor berikutnya yang dinegasikan
    }
    else {
        // error handling
        reportError("Expected Factor (ident, litaral, '(' or NOT");
        node->addChild(new ParseNode("ERROR"));
        advToken();
    }
    
    return node;
}
ParseNode* Parser::parseComponentVariable() {
    ParseNode* node = new ParseNode("<component-variable>");
    
    if (peekToken().type == TokenType::LBRACK) {
        node->addChild(match(TokenType::LBRACK));
        node->addChild(parseIndexList());
        node->addChild(match(TokenType::RBRACK));
    } 
    else if (peekToken().type == TokenType::PERIOD) {
        node->addChild(match(TokenType::PERIOD));
        node->addChild(match(TokenType::IDENT));
    }
    else {
        reportError("Expected '[' for array element or '.' for record field");
        node->addChild(new ParseNode("ERROR"));
        advToken();
    }

    return node;
}

ParseNode* Parser::parseIndexList(){
    ParseNode* node = new ParseNode("<index-list>");
    
    // ( intcon| charcon|ident)
    TokenType cur = peekToken().type;
    if (cur == TokenType::INTCON || cur == TokenType::CHARCON || cur == TokenType::IDENT) {
        node->addChild(match(cur));
    } else {
        reportError("Expected integer constant, character constant, or identifier as array index");
        node->addChild(new ParseNode("ERROR"));
        advToken();
    }
    
    // +(comma + index-list )*
    while (peekToken().type == TokenType::COMMA) {
        node->addChild(match(TokenType::COMMA));
        
        cur = peekToken().type;
        if (cur == TokenType::INTCON || cur == TokenType::CHARCON || cur == TokenType::IDENT) {
            node->addChild(match(cur));
        } else {
            reportError("Expected integer constant, character constant, or identifier as array index");
            node->addChild(new ParseNode("ERROR"));
            advToken();
        }
    }
    
    return node;
}
ParseNode* Parser::parseVariable(){
    ParseNode* node = new ParseNode("<variable>");
    TokenType cur = peekToken().type;
    if(cur == TokenType::IDENT){
        node->addChild(match(cur));
        while(peekToken().type == TokenType::LBRACK || peekToken().type == TokenType::PERIOD){
            node->addChild(parseComponentVariable());
        }
    }
    else{
        reportError("Expected Identifier for variable");
        node->addChild(new ParseNode("ERROR"));
        advToken();
    }
    return node;
    
}
ParseNode* Parser::parseRelationalOperator(){
    ParseNode* node = new ParseNode("<relational-operator>");
    node->addChild(match(peekToken().type)); // ini udah pasti eql,neq, gtr, geq, lss, leq
    return node;
}

ParseNode* Parser::parseAdditiveOperator(){
    ParseNode* node = new ParseNode("<additive-operator>");
    node->addChild(match(peekToken().type)); // ini udah pasti plus, minus, orsy
    return node;
}

ParseNode* Parser::parseMultiplicativeOperator(){
    ParseNode* node = new ParseNode("<multiplicative-operator>");
    node->addChild(match(peekToken().type));// ini udh psti times | rdiv | idiv | imod | andsy
    return node;
}