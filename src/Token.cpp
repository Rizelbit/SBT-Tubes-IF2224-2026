#include "Token.hpp"
#include <unordered_map>

Token::Token(TokenType type, std::string value, int line, int column)
    : type(type), value(value), line(line), column(column) {}

std::string Token::toString() const {
    static const std::unordered_map<TokenType, std::string> typeNames = {
        {TokenType::INTCON,      "INTCON"},
        {TokenType::REALCON,     "REALCON"},
        {TokenType::CHARCON,     "CHARCON"},
        {TokenType::STRING,      "STRING"},
        {TokenType::NOTSY,       "NOTSY"},
        {TokenType::PLUS,        "PLUS"},
        {TokenType::MINUS,       "MINUS"},
        {TokenType::TIMES,       "TIMES"},
        {TokenType::IDIV,        "IDIV"},
        {TokenType::RDIV,        "RDIV"},
        {TokenType::IMOD,        "IMOD"},
        {TokenType::ANDSY,       "ANDSY"},
        {TokenType::ORSY,        "ORSY"},
        {TokenType::EQL,         "EQL"},
        {TokenType::NEQ,         "NEQ"},
        {TokenType::GTR,         "GTR"},
        {TokenType::GEQ,         "GEQ"},
        {TokenType::LSS,         "LSS"},
        {TokenType::LEQ,         "LEQ"},
        {TokenType::LPARENT,     "LPARENT"},
        {TokenType::RPARENT,     "RPARENT"},
        {TokenType::LBRACK,      "LBRACK"},
        {TokenType::RBRACK,      "RBRACK"},
        {TokenType::COMMA,       "COMMA"},
        {TokenType::SEMICOLON,   "SEMICOLON"},
        {TokenType::PERIOD,      "PERIOD"},
        {TokenType::COLON,       "COLON"},
        {TokenType::BECOMES,     "BECOMES"},
        {TokenType::CONSTSY,     "CONSTSY"},
        {TokenType::TYPESY,      "TYPESY"},
        {TokenType::VARSY,       "VARSY"},
        {TokenType::FUNCTIONSY,  "FUNCTIONSY"},
        {TokenType::PROCEDURESY, "PROCEDURESY"},
        {TokenType::ARRAYSY,     "ARRAYSY"},
        {TokenType::RECORDSY,    "RECORDSY"},
        {TokenType::PROGRAMSY,   "PROGRAMSY"},
        {TokenType::BEGINSY,     "BEGINSY"},
        {TokenType::IFSY,        "IFSY"},
        {TokenType::CASESY,      "CASESY"},
        {TokenType::REPEATSY,    "REPEATSY"},
        {TokenType::WHILESY,     "WHILESY"},
        {TokenType::FORSY,       "FORSY"},
        {TokenType::ENDSY,       "ENDSY"},
        {TokenType::ELSESY,      "ELSESY"},
        {TokenType::UNTILSY,     "UNTILSY"},
        {TokenType::OFSY,        "OFSY"},
        {TokenType::DOSY,        "DOSY"},
        {TokenType::TOSY,        "TOSY"},
        {TokenType::DOWNTOSY,    "DOWNTOSY"},
        {TokenType::THENSY,      "THENSY"},
        {TokenType::IDENT,       "IDENT"},
        {TokenType::COMMENT,     "COMMENT"},
        {TokenType::END_OF_FILE, "END_OF_FILE"},
        {TokenType::UNKNOWN,     "UNKNOWN"},
    };

    auto it = typeNames.find(type);
    std::string name = (it != typeNames.end()) ? it->second : "UNKNOWN";

    // Token-token yang tidak perlu cetak value
    static const std::unordered_map<TokenType, bool> noValue = {
        {TokenType::PLUS, true}, {TokenType::MINUS, true}, {TokenType::TIMES, true},
        {TokenType::RDIV, true}, {TokenType::ANDSY, true}, {TokenType::ORSY, true},
        {TokenType::NOTSY, true}, {TokenType::IDIV, true}, {TokenType::IMOD, true},
        {TokenType::EQL, true}, {TokenType::NEQ, true}, {TokenType::GTR, true},
        {TokenType::GEQ, true}, {TokenType::LSS, true}, {TokenType::LEQ, true},
        {TokenType::LPARENT, true}, {TokenType::RPARENT, true},
        {TokenType::LBRACK, true}, {TokenType::RBRACK, true},
        {TokenType::COMMA, true}, {TokenType::SEMICOLON, true},
        {TokenType::PERIOD, true}, {TokenType::COLON, true}, {TokenType::BECOMES, true},
        {TokenType::CONSTSY, true}, {TokenType::TYPESY, true}, {TokenType::VARSY, true},
        {TokenType::FUNCTIONSY, true}, {TokenType::PROCEDURESY, true},
        {TokenType::ARRAYSY, true}, {TokenType::RECORDSY, true}, {TokenType::PROGRAMSY, true},
        {TokenType::BEGINSY, true}, {TokenType::IFSY, true}, {TokenType::CASESY, true},
        {TokenType::REPEATSY, true}, {TokenType::WHILESY, true}, {TokenType::FORSY, true},
        {TokenType::ENDSY, true}, {TokenType::ELSESY, true}, {TokenType::UNTILSY, true},
        {TokenType::OFSY, true}, {TokenType::DOSY, true}, {TokenType::TOSY, true},
        {TokenType::DOWNTOSY, true}, {TokenType::THENSY, true},
        {TokenType::END_OF_FILE, true}, {TokenType::COMMENT, true},
    };

    if (noValue.count(type)) {
        return name;
    }
    return name + "(" + value + ")";
}