#include "Parser.hpp"

static bool isConstantStart(TokenType type) {
    return type == TokenType::IDENT ||
           type == TokenType::INTCON ||
           type == TokenType::REALCON ||
           type == TokenType::CHARCON ||
           type == TokenType::STRING ||
           type == TokenType::PLUS ||
           type == TokenType::MINUS;
}

// Parsing semua deklarasi konstanta
ParseNode* Parser::parseConstDeclaration() {
    ParseNode* node = new ParseNode("<const-declaration>");

    node->addChild(match(TokenType::CONSTSY));

    while (peekToken().type == TokenType::IDENT) {
        node->addChild(match(TokenType::IDENT));
        node->addChild(match(TokenType::EQL));
        node->addChild(parseConstant());
        node->addChild(match(TokenType::SEMICOLON));
    }

    return node;
}

// Parsing nilai literal
ParseNode* Parser::parseConstant() {
    ParseNode* node = new ParseNode("<constant>");

    TokenType type = peekToken().type;

    if (type == TokenType::CHARCON || type == TokenType::STRING) {
        node->addChild(match(type));
    }
    else {
        if (type == TokenType::PLUS || type == TokenType::MINUS) {
            node->addChild(match(type));
            type = peekToken().type;
        }

        if (type == TokenType::IDENT ||
            type == TokenType::INTCON ||
            type == TokenType::REALCON) {
            node->addChild(match(type));
        }
        else {
            reportError("Expected constant");
            node->addChild(new ParseNode("ERROR"));
            advToken();
        }
    }

    return node;
}

// Parsing tipe baru
ParseNode* Parser::parseTypeDeclaration() {
    ParseNode* node = new ParseNode("<type-declaration>");

    node->addChild(match(TokenType::TYPESY));

    while (peekToken().type == TokenType::IDENT) {
        node->addChild(match(TokenType::IDENT));
        node->addChild(match(TokenType::EQL));
        node->addChild(parseType());
        node->addChild(match(TokenType::SEMICOLON));
    }

    return node;
}

// Deklarasi variabel
ParseNode* Parser::parseVarDeclaration() {
    ParseNode* node = new ParseNode("<var-declaration>");

    node->addChild(match(TokenType::VARSY));

    while (peekToken().type == TokenType::IDENT) {
        node->addChild(parseIdentifierList());
        node->addChild(match(TokenType::COLON));
        node->addChild(parseType());
        node->addChild(match(TokenType::SEMICOLON));
    }

    return node;
}

// Parsing daftar variabel
ParseNode* Parser::parseIdentifierList() {
    ParseNode* node = new ParseNode("<identifier-list>");

    node->addChild(match(TokenType::IDENT));

    while (peekToken().type == TokenType::COMMA) {
        node->addChild(match(TokenType::COMMA));
        node->addChild(match(TokenType::IDENT));
    }

    return node;
}

// Parsing tipe data
ParseNode* Parser::parseType() {
    ParseNode* node = new ParseNode("<type>");

    TokenType type = peekToken().type;

    if (type == TokenType::ARRAYSY) {
        node->addChild(parseArrayType());
    }
    else if (type == TokenType::RECORDSY) {
        node->addChild(parseRecordType());
    }
    else if (type == TokenType::LPARENT) {
        node->addChild(parseEnumerated());
    }
    else if (type == TokenType::IDENT) {
        if (peekToken(1).type == TokenType::PERIOD &&
            peekToken(2).type == TokenType::PERIOD) {
            node->addChild(parseRange());
        }
        else {
            node->addChild(match(TokenType::IDENT));
        }
    }
    else if (isConstantStart(type)) {
        node->addChild(parseRange());
    }
    else {
        reportError("Expected type");
        node->addChild(new ParseNode("ERROR"));
        advToken();
    }

    return node;
}

// Parsing tipe array
ParseNode* Parser::parseArrayType() {
    ParseNode* node = new ParseNode("<array-type>");

    node->addChild(match(TokenType::ARRAYSY));
    node->addChild(match(TokenType::LBRACK));

    if (peekToken().type == TokenType::IDENT &&
        peekToken(1).type != TokenType::PERIOD) {
        node->addChild(match(TokenType::IDENT));
    }
    else {
        node->addChild(parseRange());
    }

    node->addChild(match(TokenType::RBRACK));
    node->addChild(match(TokenType::OFSY));
    node->addChild(parseType());

    return node;
}

// Parsing range
ParseNode* Parser::parseRange() {
    ParseNode* node = new ParseNode("<range>");

    node->addChild(parseConstant());
    node->addChild(match(TokenType::PERIOD));
    node->addChild(match(TokenType::PERIOD));
    node->addChild(parseConstant());

    return node;
}

// Parsing tipe enum 
ParseNode* Parser::parseEnumerated() {
    ParseNode* node = new ParseNode("<enumerated>");

    node->addChild(match(TokenType::LPARENT));
    node->addChild(match(TokenType::IDENT));

    while (peekToken().type == TokenType::COMMA) {
        node->addChild(match(TokenType::COMMA));
        node->addChild(match(TokenType::IDENT));
    }

    node->addChild(match(TokenType::RPARENT));

    return node;
}

// Parsing tipe record
ParseNode* Parser::parseRecordType() {
    ParseNode* node = new ParseNode("<record-type>");

    node->addChild(match(TokenType::RECORDSY));
    node->addChild(parseFieldList());
    node->addChild(match(TokenType::ENDSY));

    return node;
}

// Parsing semua field dalam record
ParseNode* Parser::parseFieldList() {
    ParseNode* node = new ParseNode("<field-list>");

    node->addChild(parseFieldPart());

    while (peekToken().type == TokenType::SEMICOLON &&
           peekToken(1).type != TokenType::ENDSY) {
        node->addChild(match(TokenType::SEMICOLON));
        node->addChild(parseFieldPart());
    }

    if (peekToken().type == TokenType::SEMICOLON) {
        node->addChild(match(TokenType::SEMICOLON));
    }

    return node;
}

// Parsing satu field dalam record
ParseNode* Parser::parseFieldPart() {
    ParseNode* node = new ParseNode("<field-part>");

    node->addChild(parseIdentifierList());
    node->addChild(match(TokenType::COLON));
    node->addChild(parseType());

    return node;
}