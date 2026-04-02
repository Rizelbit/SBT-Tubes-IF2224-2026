#include "Lexer.hpp"
#include <cctype>

Token Lexer::lexLiteral() {
	int startLine = currentLine;
	int startColumn = currentColumn;
	std::string lexeme = "";

	if (currentChar == '-') {
		lexeme += currentChar;
		advance();
	}

	if (std::isdigit(static_cast<unsigned char>(currentChar))) {
		while (std::isdigit(static_cast<unsigned char>(currentChar))) {
			lexeme += currentChar;
			advance();
		}

		if (currentChar == '.') {
			lexeme += currentChar;
			advance();

			if (std::isdigit(static_cast<unsigned char>(currentChar))) {
				while (std::isdigit(static_cast<unsigned char>(currentChar))) {
					lexeme += currentChar;
					advance();
				}
				return Token(TokenType::REALCON, lexeme, startLine, startColumn);
			} else {
				retract();
				lexeme.pop_back();
				currentChar = '.';
				retract();
				advance();
				return Token(TokenType::INTCON, lexeme, startLine, startColumn);
			}
		}

		return Token(TokenType::INTCON, lexeme, startLine, startColumn);
	}

	if (currentChar == '\'') {
		lexeme += currentChar;
		advance();

		std::string content = "";

		while (true) {
			if (currentChar == '\0' || currentChar == '\n' || currentChar == '\r') {
				return Token(TokenType::UNKNOWN, "Unterminated string", startLine, startColumn);
			}

			if (currentChar == '\'') {
				lexeme += currentChar;
				advance();

				if (currentChar == '\'') {
					lexeme += currentChar;
					content += '\'';
					advance();
				} else {
					break;
				}
			} else {
				lexeme += currentChar;
				content += currentChar;
				advance();
			}
		}

		if (content.size() == 1) {
			return Token(TokenType::CHARCON, content, startLine, startColumn);
		}
		return Token(TokenType::STRING, content, startLine, startColumn);
	}

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
