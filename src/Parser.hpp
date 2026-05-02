#pragma once

#include "Lexer.hpp"
#include "ParseNode.hpp"
#include <vector>
#include <string>

class Parser {
private:
    std::vector<Token> tokens;
    size_t currTokenIdx;
    bool hasError;

    Token peekToken(int offset = 0) const;
    void advToken();
    void reportError(const std::string& message);
    ParseNode* match(TokenType expectedType);

public:
    Parser(Lexer& lexer);
    ParseNode* parse();
    bool isSuccess() const;

    // Core & Structure
    ParseNode* parseProgram();
    ParseNode* parseProgramHeader();
    ParseNode* parseDeclarationPart();
    ParseNode* parseBlock();
    ParseNode* parseCompoundStatement();
    ParseNode* parseStatement();
    ParseNode* parseStatementList();

    // Declarations & Types
    ParseNode* parseConstDeclaration();
    ParseNode* parseConstant();
    ParseNode* parseTypeDeclaration();
    ParseNode* parseVarDeclaration();
    ParseNode* parseIdentifierList();
    ParseNode* parseType();
    ParseNode* parseArrayType();
    ParseNode* parseRange();
    ParseNode* parseEnumerated();
    ParseNode* parseRecordType();
    ParseNode* parseFieldList();
    ParseNode* parseFieldPart();

    // Control Flow & Subprograms
    ParseNode* parseSubprogramDeclaration();
    ParseNode* parseProcedureDeclaration();
    ParseNode* parseFunctionDeclaration();
    ParseNode* parseFormalParameterList();
    ParseNode* parseParameterGroup();
    ParseNode* parseIfStatement();
    ParseNode* parseCaseStatement();
    ParseNode* parseCaseBlock();
    ParseNode* parseWhileStatement();
    ParseNode* parseRepeatStatement();
    ParseNode* parseForStatement();

    // Expressions
    ParseNode* parseAssignmentStatement();
    ParseNode* parseProcedureFunctionCall();
    ParseNode* parseParameterList();
    ParseNode* parseExpression();
    ParseNode* parseSimpleExpression();
    ParseNode* parseTerm();
    ParseNode* parseFactor();
    ParseNode* parseRelationalOperator();
    ParseNode* parseAdditiveOperator();
    ParseNode* parseMultiplicativeOperator();
};