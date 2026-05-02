#include "Parser.hpp"

Parser::Parser(Lexer& lexer) : currTokenIdx(0), hasError(false) {
    std::vector<Token> allTokens = lexer.tokenizeAll();

    for (const auto& t : allTokens) {
        if (t.type == TokenType::UNKNOWN) {
            std::cerr << "Lexical Error at Line " << t.line << ", Col " << t.column << ": " << t.value << "\n";
        } else if (t.type != TokenType::COMMENT) {
            tokens.push_back(t);
        }
    }
}

ParseNode* Parser::parse() {
    return parseProgram();
}

bool Parser::isSuccess() const {
    return !hasError;
}

Token Parser::peekToken(int offset) const {
    if (currTokenIdx + offset < tokens.size()) {
        return tokens[currTokenIdx + offset];
    }

    return tokens.back();
}

void Parser::advToken() {
    if (currTokenIdx < tokens.size() - 1) {
        currTokenIdx++;
    }
}

void Parser::reportError(const std::string& message) {
    if (!hasError) {
        std::cerr << "Syntax Error at Line " << peekToken().line << ", Col " << peekToken().column << ": " << message << "\n";
        hasError = true;
    }
}

ParseNode* Parser::match(TokenType expectedType) {
    Token curr = peekToken();
    if (curr.type == expectedType) {
        ParseNode* node = new ParseNode(curr.toString());
        advToken();

        return node;
    } else {
        reportError("Unexpected token " + curr.toString());
        return new ParseNode("ERROR");
    }
}