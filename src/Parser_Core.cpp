#include "Parser.hpp"

ParseNode* Parser::parseProgram() {
    ParseNode* node = new ParseNode("<program>");

    node->addChild(parseProgramHeader());
    node->addChild(parseDeclarationPart());
    node->addChild(parseCompoundStatement());
    node->addChild(match(TokenType::PERIOD));

    return node;
}

ParseNode* Parser::parseProgramHeader() {
    ParseNode* node = new ParseNode("<program-header>");

    node->addChild(match(TokenType::PROGRAMSY));
    node->addChild(match(TokenType::IDENT));
    node->addChild(match(TokenType::SEMICOLON));

    return node;
}

ParseNode* Parser::parseDeclarationPart(){
    ParseNode* node = new ParseNode("<declaration-part>");

    while (peekToken().type == TokenType::CONSTSY) {
        node->addChild(parseConstDeclaration());
    }
    while (peekToken().type == TokenType::TYPESY) {
        node->addChild(parseTypeDeclaration());
    }
    while (peekToken().type == TokenType::VARSY) {
        node->addChild(parseVarDeclaration());
    }
    while (peekToken().type == TokenType::PROCEDURESY || peekToken().type == TokenType::FUNCTIONSY) {
        node->addChild(parseSubprogramDeclaration());
    }

    return node;
}

ParseNode* Parser::parseBlock(){
    ParseNode* node = new ParseNode("<block>");

    node->addChild(parseDeclarationPart());
    node->addChild(parseCompoundStatement());

    return node;
}

ParseNode* Parser::parseCompoundStatement(){
    ParseNode* node = new ParseNode("<compound-statement>");

    node->addChild(match(TokenType::BEGINSY));
    node->addChild(parseStatementList());
    node->addChild(match(TokenType::ENDSY));

    return node;
}


ParseNode* Parser::parseStatement(){
    ParseNode* node = new ParseNode("<statement>");
    TokenType type = peekToken().type;

    if (type == TokenType::IDENT) {
        TokenType nextType = peekToken(1).type;
        if (nextType == TokenType::BECOMES || 
            nextType == TokenType::LBRACK || 
            nextType == TokenType::PERIOD) {
            node->addChild(parseAssignmentStatement());
        } else {
            node->addChild(parseProcedureFunctionCall());
        }
    } else if (type == TokenType::IFSY) {
        node->addChild(parseIfStatement());
    } else if (type == TokenType::CASESY) {
        node->addChild(parseCaseStatement());
    } else if (type == TokenType::WHILESY) {
        node->addChild(parseWhileStatement());
    } else if (type == TokenType::REPEATSY) {
        node->addChild(parseRepeatStatement());
    } else if (type == TokenType::FORSY) {
        node->addChild(parseForStatement());
    }

    return node;
}

ParseNode* Parser::parseStatementList(){
    ParseNode* node = new ParseNode("<statement-list>");
    
    node->addChild(parseStatement());
    
    while (peekToken().type == TokenType::SEMICOLON) {
        node->addChild(match(TokenType::SEMICOLON));
        node->addChild(parseStatement());
    }
    
    return node;
}