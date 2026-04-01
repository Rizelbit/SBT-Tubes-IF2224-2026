#include "Lexer.hpp"
#include <cctype>

Token Lexer::lexLiteral() {
	int startLine = currentLine;
	int startColumn = currentColumn;
	std::string lexeme = "";

	// Number literal: INTCON (digit+) atau REALCON (digit+ '.' digit+)
	if (std::isdigit(static_cast<unsigned char>(currentChar))) {
		while (std::isdigit(static_cast<unsigned char>(currentChar))) {
			lexeme += currentChar;
			advance();
		}

		if (currentChar == '.' && std::isdigit(static_cast<unsigned char>(peek()))) {
			lexeme += currentChar;
			advance();

			while (std::isdigit(static_cast<unsigned char>(currentChar))) {
				lexeme += currentChar;
				advance();
			}

			return Token(TokenType::REALCON, lexeme, startLine, startColumn);
		}

		return Token(TokenType::INTCON, lexeme, startLine, startColumn);
	}

	// Single-quoted literal: CHARCON untuk karakter tunggal, STRING untuk banyak karakter.
	if (currentChar == '\'') {
		lexeme += currentChar;
		advance();

		std::string content = "";
		while (currentChar != '\'' && currentChar != '\0' && currentChar != '\n' && currentChar != '\r') {
			content += currentChar;
			lexeme += currentChar;
			advance();
		}

		if (currentChar == '\'') {
			lexeme += currentChar;
			advance();

			if (content.size() == 1) {
				return Token(TokenType::CHARCON, lexeme, startLine, startColumn);
			}

			return Token(TokenType::STRING, lexeme, startLine, startColumn);
		}

		return Token(TokenType::UNKNOWN, lexeme, startLine, startColumn);
	}

	// Double-quoted literal selalu STRING.
	if (currentChar == '"') {
		lexeme += currentChar;
		advance();

		while (currentChar != '"' && currentChar != '\0' && currentChar != '\n' && currentChar != '\r') {
			lexeme += currentChar;
			advance();
		}

		if (currentChar == '"') {
			lexeme += currentChar;
			advance();
			return Token(TokenType::STRING, lexeme, startLine, startColumn);
		}

		return Token(TokenType::UNKNOWN, lexeme, startLine, startColumn);
	}
	char c = currentChar;
	advance();
	return Token(TokenType::UNKNOWN, std::string(1, c), startLine, startColumn);
}
