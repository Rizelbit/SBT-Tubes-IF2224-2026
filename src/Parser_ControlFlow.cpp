#include "Parser.hpp"

ParseNode* Parser::parseSubprogramDeclaration(){
    ParseNode* node = new ParseNode("<subprogram-declaration>");

    if(peekToken().type == TokenType::PROCEDURESY){
        node->addChild(parseProcedureDeclaration());
    } else if (peekToken().type == TokenType::FUNCTIONSY){
        node->addChild(parseFunctionDeclaration());
    } else {
        reportError("Expected 'procedure' or 'function' keyword");
    }

    return node;
}

ParseNode* Parser::parseProcedureDeclaration(){
    ParseNode* node = new ParseNode("<procedure-declaration>");

    node->addChild(match(TokenType::PROCEDURESY));
    node->addChild(match(TokenType::IDENT));

    if(peekToken().type == TokenType::LPARENT){
        node->addChild(parseFormalParameterList());
    }

    node->addChild(match(TokenType::SEMICOLON));
    node->addChild(parseBlock());
    node->addChild(match(TokenType::SEMICOLON));

    return node;
}

ParseNode* Parser::parseFunctionDeclaration() {
    ParseNode* node = new ParseNode("<function-declaration>");

    node->addChild(match(TokenType::FUNCTIONSY));
    node->addChild(match(TokenType::IDENT));

    if (peekToken().type == TokenType::LPARENT) {
        node->addChild(parseFormalParameterList());
    }

    node->addChild(match(TokenType::COLON));
    node->addChild(match(TokenType::IDENT));

    node->addChild(match(TokenType::SEMICOLON));
    node->addChild(parseBlock());
    node->addChild(match(TokenType::SEMICOLON));

    return node;
}

ParseNode* Parser::parseFormalParameterList() {
    ParseNode* node = new ParseNode("<formal-parameter-list>");

    node->addChild(match(TokenType::LPARENT));
    node->addChild(parseParameterGroup());

    while (peekToken().type == TokenType::SEMICOLON) {
        node->addChild(match(TokenType::SEMICOLON));
        node->addChild(parseParameterGroup());
    }

    node->addChild(match(TokenType::RPARENT));

    return node;
}

ParseNode* Parser::parseParameterGroup() {
    ParseNode* node = new ParseNode("<parameter-group>");

    node->addChild(parseIdentifierList());
    node->addChild(match(TokenType::COLON));

    if (peekToken().type == TokenType::ARRAYSY) {
        node->addChild(parseArrayType());
    } else {
        node->addChild(match(TokenType::IDENT));
    }

    return node;
}

ParseNode* Parser::parseIfStatement() {
    ParseNode* node = new ParseNode("<if-statement>");

    node->addChild(match(TokenType::IFSY));
    node->addChild(parseExpression());
    node->addChild(match(TokenType::THENSY));
    node->addChild(parseStatement());

    if (peekToken().type == TokenType::ELSESY) {
        node->addChild(match(TokenType::ELSESY));
        node->addChild(parseStatement());
    }

    return node;
}

ParseNode* Parser::parseCaseStatement() {
    ParseNode* node = new ParseNode("<case-statement>");

    node->addChild(match(TokenType::CASESY));
    node->addChild(parseExpression());
    node->addChild(match(TokenType::OFSY));
    node->addChild(parseCaseBlock());
    node->addChild(match(TokenType::ENDSY));

    return node;
}

ParseNode* Parser::parseCaseBlock() {
    ParseNode* node = new ParseNode("<case-block>");

    node->addChild(parseConstant());

    while (peekToken().type == TokenType::COMMA) {
        node->addChild(match(TokenType::COMMA));
        node->addChild(parseConstant());
    }

    node->addChild(match(TokenType::COLON));
    node->addChild(parseStatement());

    while (peekToken().type == TokenType::SEMICOLON) {
        TokenType nextType = peekToken(1).type;
        bool canStartCaseBlock = (
            nextType == TokenType::INTCON ||
            nextType == TokenType::REALCON ||
            nextType == TokenType::CHARCON ||
            nextType == TokenType::STRING ||
            nextType == TokenType::IDENT ||
            nextType == TokenType::PLUS ||
            nextType == TokenType::MINUS
        );

        if (!canStartCaseBlock) break;

        node->addChild(match(TokenType::SEMICOLON));
        node->addChild(parseCaseBlock());
        break;
    }

    return node;
}

ParseNode* Parser::parseWhileStatement() {
    ParseNode* node = new ParseNode("<while-statement>");

    node->addChild(match(TokenType::WHILESY));
    node->addChild(parseExpression());
    node->addChild(match(TokenType::DOSY));
    node->addChild(parseStatement());

    return node;
}

ParseNode* Parser::parseRepeatStatement() {
    ParseNode* node = new ParseNode("<repeat-statement>");

    node->addChild(match(TokenType::REPEATSY));
    node->addChild(parseStatementList());
    node->addChild(match(TokenType::UNTILSY));
    node->addChild(parseExpression());

    return node;
}

ParseNode* Parser::parseForStatement() {
    ParseNode* node = new ParseNode("<for-statement>");

    node->addChild(match(TokenType::FORSY));
    node->addChild(match(TokenType::IDENT));
    node->addChild(match(TokenType::BECOMES));
    node->addChild(parseExpression());

    // (tosy | downtosy) — arah iterasi, wajib salah satu
    if (peekToken().type == TokenType::TOSY) {
        node->addChild(match(TokenType::TOSY));
    } else if (peekToken().type == TokenType::DOWNTOSY) {
        node->addChild(match(TokenType::DOWNTOSY));
    } else {
        reportError("Expected 'to' or 'downto' in for-statement, got: " + peekToken().toString());
        // Graceful fallback: buat node error tapi jangan crash
        node->addChild(new ParseNode("ERROR(expected to/downto)"));
    }

    node->addChild(parseExpression());
    node->addChild(match(TokenType::DOSY));
    node->addChild(parseStatement());

    return node;
}