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

}

ParseNode* Parser::parseSimpleExpression(){

}

ParseNode* Parser::parseTerm(){

}

ParseNode* Parser::parseFactor(){

}

ParseNode* Parser::parseRelationalOperator(){

}

ParseNode* Parser::parseAdditiveOperator(){

}

ParseNode* Parser::parseMultiplicativeOperator(){

}