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
				retract();                         // put back the non-digit char
				lexeme.pop_back();                 // remove '.' from lexeme
				// Now we need to also retract the '.' itself.
				// After retract(), currentChar is invalid.  We need the
				// stream to now yield '.' on the next advance().
				// Put '.' back into the stream.
				currentChar = '.';
				retract();                         // puts '.' back
				// Now advance() to set currentChar to '.' with correct position
				advance();
				return Token(TokenType::INTCON, lexeme, startLine, startColumn);
			}
		}

		return Token(TokenType::INTCON, lexeme, startLine, startColumn);
	}

	// ----------------------------------------------------------------
	// String / Char literal DFA (single-quote delimited)
	//
	// States:
	//   S_OPEN    — opening quote consumed
	//   S_BODY    — reading characters inside the string
	//   S_QUOTE   — saw a quote inside; could be escape ('') or closing
	//   S_DONE    — string complete
	//   S_ERROR   — unterminated (hit \n, \r, or EOF)
	// ----------------------------------------------------------------
	if (currentChar == '\'') {
		lexeme += currentChar;
		advance(); // consume opening quote

		std::string content = "";

		// S_BODY loop
		while (true) {
			// S_ERROR check
			if (currentChar == '\0' || currentChar == '\n' || currentChar == '\r') {
				return Token(TokenType::UNKNOWN, "Unterminated string", startLine, startColumn);
			}

			if (currentChar == '\'') {
				// S_QUOTE: could be closing quote or start of '' escape
				lexeme += currentChar;
				advance(); // consume this quote, look at next char

				if (currentChar == '\'') {
					// '' escape — the second quote is part of the escape
					lexeme += currentChar;
					content += '\'';   // escaped quote → literal '
					advance();         // consume second quote, stay in S_BODY
				} else {
					// Closing quote — done.  currentChar is already the
					// next char after the string (no retract needed).
					break;
				}
			} else {
				// Regular character
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

	// ----------------------------------------------------------------
	// Double-quote string literal (DFA)
	// ----------------------------------------------------------------
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

	// Fallback — shouldn't be reached in normal dispatch
	char c = currentChar;
	advance();
	return Token(TokenType::UNKNOWN, std::string(1, c), startLine, startColumn);
}
