#include "Parser.hpp"

ParseNode* Parser::parseAssignmentStatement(){
    ParseNode* node = new ParseNode("AssignmentStatement");
    node->addChild(match(TokenType::IDENT));
    node->addChild(match(TokenType::BECOMES)); // :=
    node->addChild(parseExpression());
    return node;
}

ParseNode* Parser::parseProcedureFunctionCall(){

}
ParseNode* Parser::parseParameterList(){

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