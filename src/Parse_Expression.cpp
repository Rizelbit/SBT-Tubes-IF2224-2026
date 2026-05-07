#include "Parser.hpp"

ParseNode* Parser::parseAssignmentStatement(){
    ParseNode* node = new ParseNode("AssignmentStatement");
    node->addChild(match(TokenType::IDENT));
    node->addChild(match(TokenType::BECOMES)); // :=
    node->addChild(parseExpression());
    return node;
}

ParseNode* Parser::parseProcedureFunctionCall(){
    ParseNode* node = new ParseNode("ProcedureFunctionCall");
    
    // harus diawali dengan identifier (nama fungsi atau prosedur)
    node->addChild(match(TokenType::IDENT));
    
    // Opsional: Cek apakah ada parameter yang dikirim (ditandai dengan kurung buka)
    if (peekToken().type == TokenType::LPARENT) {
        node->addChild(match(TokenType::LPARENT));
        node->addChild(parseParameterList()); // Parse isi parameter
        node->addChild(match(TokenType::RPARENT));
    }
    
    return node;
}
ParseNode* Parser::parseParameterList(){
    ParseNode* node = new ParseNode("ParameterList");
    
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
    ParseNode* node = new ParseNode("Expression");
    
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
    ParseNode* node = new ParseNode("SimpleExpression");
    
    // Cek opsional unary sign di awal ekspresi (misal: -5 atau +10)
    TokenType cur = peekToken().type;
    if (cur == TokenType::PLUS || cur == TokenType::MINUS) {
        node->addChild(match(cur));
    }
    
    // Harus selalu ada term
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

ParseNode* Parser::parseTerm(){
    ParseNode* node = new ParseNode("Term");
    node->addChild(parseFactor());

    // slama ada operator perkalian ( *, /, div, mod, AND )
    while (peekToken().type == TokenType::TIMES || peekToken().type == TokenType::RDIV ||
    peekToken().type == TokenType::IDIV  || peekToken().type == TokenType::IMOD || 
    peekToken().type == TokenType::ANDSY) {
        
        node->addChild(parseMultiplicativeOperator());
        node->addChild(parseFactor());
    }
}

ParseNode* Parser::parseFactor(){
    ParseNode* node = new ParseNode("Factor");
    TokenType cur = peekToken().type;
    
    // Jika itu adalah identifier (bisa variabel sederhana, atau pemanggilan fungsi)
    if (cur == TokenType::IDENT) {
        // Lookahead: Jika ada kurung buka, besar kemungkinan ini pemanggilan fungsi.
        if (peekToken(1).type == TokenType::LPARENT) {
            node->addChild(parseProcedureFunctionCall());
        } else {
            // Identifier biasa (atau bisa diperluas untuk record/array bila berlanjut)
            node->addChild(match(TokenType::IDENT));
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
        std::cout << "ERROR" << std::endl;
    }
    
    return node;
}

ParseNode* Parser::parseRelationalOperator(){
    ParseNode* node = new ParseNode("RelationalOperator");
    node->addChild(match(peekToken().type)); 
    return node;
}

ParseNode* Parser::parseAdditiveOperator(){
    ParseNode* node = new ParseNode("AdditiveOperator");
    node->addChild(match(peekToken().type)); 
    return node;
}

ParseNode* Parser::parseMultiplicativeOperator(){
    ParseNode* node = new ParseNode("MultiplicativeOperator");
    node->addChild(match(peekToken().type));
    return node;
}