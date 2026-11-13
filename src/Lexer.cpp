#include "bminor/Lexer.h"

#include <string>
#include <utility>

namespace bminor {
    namespace {
        bool isIdentifierStart(char c) {
            return (c >= 'a' && c <= 'z') ||
                   (c >= 'A' && c <= 'Z') ||
                   c == '_';
        }

        bool isIdentifierPart(char c) {
            return isIdentifierStart(c) ||
                   (c >= '0' && c <= '9');
        }

        bool isDigit(char c) {
            return c >= '0' && c <= '9';
        }

        bool isPrintableAscii(char c) {
            const auto byte = static_cast<unsigned char>(c);
            return byte >= 32 && byte <= 126;
        }

        bool isWhitespace(char c) {
            return c == ' ' ||
                   c == '\t' ||
                   c == '\n' ||
                   c == '\r';
        }

        bool isSingleCharacterOperator(char c) {
            switch (c) {
                case '+':
                case '-':
                case '*':
                case '/':
                case '%':
                case '^':
                case '<':
                case '>':
                case '!':
                case '=':
                    return true;

                default:
                    return false;
            }
        }

        bool isDelimiter(char c) {
            switch (c) {
                case '(':
                case ')':
                case '[':
                case ']':
                case '{':
                case '}':
                case ';':
                case ',':
                case ':':
                    return true;

                default:
                    return false;
            }
        }

        TokenKind classifyWord(const std::string &word) {
            if (word == "true" || word == "false") {
                return TokenKind::BooleanLiteral;
            }

            if (word == "array" || word == "boolean" ||
                word == "char" || word == "else" ||
                word == "for" || word == "function" ||
                word == "if" || word == "integer" ||
                word == "print" || word == "return" ||
                word == "string" || word == "void" ||
                word == "while") {
                return TokenKind::Keyword;
            }

            return TokenKind::Identifier;
        }
    }


    Lexer::Lexer(std::string source, std::string fileName)
        : cursor_(std::move(source), std::move(fileName)) {
    }


    Token Lexer::nextToken() {
        // whitespace
        while (!cursor_.atEnd() &&
               isWhitespace(cursor_.peek())) {
            cursor_.advance();
        }

        SourceLocation start = cursor_.location();

        // EndOfFile
        if (cursor_.atEnd()) {
            return {TokenKind::EndOfFile, "", start};
        }

        // Identifier, Keyword, BooleanLiteral
        if (isIdentifierStart(cursor_.peek())) {
            std::string lexeme;

            do {
                lexeme += cursor_.advance();
            } while (!cursor_.atEnd() &&
                     isIdentifierPart(cursor_.peek()));

            if (lexeme.size() > 256) {
                return {
                    TokenKind::Invalid,
                    lexeme,
                    start,
                    "Identifier exceeds 256 characters"
                };
            }

            return {classifyWord(lexeme), lexeme, start};
        }

        // IntegerLiteral
        if (isDigit(cursor_.peek())) {
            std::string lexeme;

            do {
                lexeme += cursor_.advance();
            } while (!cursor_.atEnd() &&
                     isDigit(cursor_.peek()));

            return {TokenKind::IntegerLiteral, lexeme, start};
        }


        // StringLiteral
        if (cursor_.peek() == '"') {
            std::string lexeme;
            std::size_t contentLength = 0;
            bool valid = true;
            std::string errorMessage;

            lexeme += cursor_.advance();

            while (!cursor_.atEnd()) {
                const char c = cursor_.peek();

                if (c == '\n' || c == '\r') {
                    break;
                }

                if (c == '"') {
                    lexeme += cursor_.advance();

                    return {
                        valid
                            ? TokenKind::StringLiteral
                            : TokenKind::Invalid,
                        lexeme,
                        start,
                        errorMessage
                    };
                }

                if (c == '\\') {
                    lexeme += cursor_.advance();

                    if (cursor_.atEnd() ||
                        cursor_.peek() == '\n' ||
                        cursor_.peek() == '\r') {
                        break;
                    }

                    const char escaped = cursor_.advance();
                    lexeme += escaped;

                    if (!isPrintableAscii(escaped)) {
                        valid = false;

                        if (errorMessage.empty()) {
                            errorMessage =
                                    "Invalid character in string literal";
                        }
                    }
                } else {
                    lexeme += cursor_.advance();

                    if (!isPrintableAscii(c)) {
                        valid = false;

                        if (errorMessage.empty()) {
                            errorMessage =
                                    "Invalid character in string literal";
                        }
                    }
                }

                ++contentLength;

                if (contentLength > 256) {
                    valid = false;

                    if (errorMessage.empty()) {
                        errorMessage =
                                "String literal exceeds 256 characters";
                    }
                }
            }

            return {
                TokenKind::Invalid,
                lexeme,
                start,
                "Unterminated string literal"
            };
        }


        // CharacterLiteral
        if (cursor_.peek() == '\'') {
            std::string lexeme;
            std::size_t contentLength = 0;
            bool valid = true;
            std::string errorMessage;

            lexeme += cursor_.advance();

            while (!cursor_.atEnd()) {
                const char c = cursor_.peek();

                if (c == '\n' || c == '\r') {
                    break;
                }

                if (c == '\'') {
                    lexeme += cursor_.advance();

                    if (contentLength != 1) {
                        valid = false;

                        if (errorMessage.empty()) {
                            errorMessage =
                                    "Character literal must contain exactly one character";
                        }
                    }

                    return {
                        valid
                            ? TokenKind::CharacterLiteral
                            : TokenKind::Invalid,
                        lexeme,
                        start,
                        errorMessage
                    };
                }

                if (c == '\\') {
                    lexeme += cursor_.advance();

                    if (cursor_.atEnd() ||
                        cursor_.peek() == '\n' ||
                        cursor_.peek() == '\r') {
                        break;
                    }

                    const char escaped = cursor_.advance();
                    lexeme += escaped;

                    if (!isPrintableAscii(escaped)) {
                        valid = false;

                        if (errorMessage.empty()) {
                            errorMessage =
                                    "Invalid character in character literal";
                        }
                    }
                } else {
                    lexeme += cursor_.advance();

                    if (!isPrintableAscii(c)) {
                        valid = false;

                        if (errorMessage.empty()) {
                            errorMessage =
                                    "Invalid character in character literal";
                        }
                    }
                }

                ++contentLength;

                if (contentLength > 1) {
                    valid = false;

                    if (errorMessage.empty()) {
                        errorMessage =
                                "Character literal must contain exactly one character";
                    }
                }
            }

            return {
                TokenKind::Invalid,
                lexeme,
                start,
                "Unterminated character literal"
            };
        }

        // LineComment
        if (cursor_.peek() == '/' && cursor_.peek(1) == '/') {
            std::string lexeme;

            lexeme += cursor_.advance();
            lexeme += cursor_.advance();

            while (!cursor_.atEnd() &&
                   cursor_.peek() != '\n' &&
                   cursor_.peek() != '\r') {
                lexeme += cursor_.advance();
            }

            return {TokenKind::LineComment, lexeme, start};
        }

        // BlockComment
        if (cursor_.peek() == '/' &&
            cursor_.peek(1) == '*') {
            std::string lexeme;

            lexeme += cursor_.advance();
            lexeme += cursor_.advance();

            while (!cursor_.atEnd()) {
                if (cursor_.peek() == '*' &&
                    cursor_.peek(1) == '/') {
                    lexeme += cursor_.advance();
                    lexeme += cursor_.advance();

                    return {
                        TokenKind::BlockComment,
                        lexeme,
                        start
                    };
                }

                lexeme += cursor_.advance();
            }

            return {
                TokenKind::Invalid,
                lexeme,
                start,
                "Unterminated block comment"
            };
        }


        // two-character operators
        const char first = cursor_.peek();
        const char second = cursor_.peek(1);

        if ((first == '+' && second == '+') ||
            (first == '-' && second == '-') ||
            (first == '<' && second == '=') ||
            (first == '>' && second == '=') ||
            (first == '=' && second == '=') ||
            (first == '!' && second == '=') ||
            (first == '&' && second == '&') ||
            (first == '|' && second == '|')) {
            std::string lexeme;
            lexeme += cursor_.advance();
            lexeme += cursor_.advance();

            return {TokenKind::Operator, lexeme, start};
        }

        // single-character operators
        if (isSingleCharacterOperator(first)) {
            return {
                TokenKind::Operator,
                std::string(1, cursor_.advance()),
                start
            };
        }

        // Delimiter
        if (isDelimiter(first)) {
            return {
                TokenKind::Delimiter,
                std::string(1, cursor_.advance()),
                start
            };
        }


        // Unsupported character
        return {
            TokenKind::Invalid,
            std::string(1, cursor_.advance()),
            start,
            "Unexpected character"
        };

    }
}
