
#include "bminor/Lexer.h"

#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <string>

struct Expected {
    bminor::TokenKind kind;
    std::string lexeme;
    std::string error = "";
    std::size_t line = 0;
    std::size_t column = 0;
};

bool check(const std::string& name,
           const std::string& source,
           std::initializer_list<Expected> expected) {

    bminor::Lexer lexer(source, "test.bminor");
    std::size_t index = 0;

    for (const auto& want : expected) {
        ++index;
        auto got = lexer.nextToken();

        if (got.kind != want.kind ||
            got.lexeme != want.lexeme ||
            got.errorMessage != want.error ||
            (want.line != 0 &&
             got.location.line != want.line) ||
            (want.column != 0 &&
             got.location.column != want.column)) {

            std::cerr << "FAIL: " << name
                      << " at token " << index
                      << " (got lexeme: [" << got.lexeme
                      << "], error: [" << got.errorMessage
                      << "])\n";
            return false;
        }
    }

    auto eof = lexer.nextToken();

    if (eof.kind != bminor::TokenKind::EndOfFile ||
        eof.location.offset != source.size() ||
        !eof.lexeme.empty() ||
        !eof.errorMessage.empty()) {

        std::cerr << "FAIL: " << name << " at EOF\n";
        return false;
    }

    std::cout << "PASS: " << name
              << " (" << index << " tokens)\n";
    return true;
}

int main() {
    using K = bminor::TokenKind;

    if (!check("program", R"(// note
main: function integer () = {
    x_1: integer = 42;
    print "Hi\n", '\q'; /* block */
    return x_1;
})", {
        {K::LineComment, "// note", "", 1, 1},
        {K::Identifier, "main", "", 2, 1},
        {K::Delimiter, ":"},
        {K::Keyword, "function"},
        {K::Keyword, "integer"},
        {K::Delimiter, "("},
        {K::Delimiter, ")"},
        {K::Operator, "="},
        {K::Delimiter, "{"},
        {K::Identifier, "x_1", "", 3, 5},
        {K::Delimiter, ":"},
        {K::Keyword, "integer"},
        {K::Operator, "="},
        {K::IntegerLiteral, "42"},
        {K::Delimiter, ";"},
        {K::Keyword, "print", "", 4, 5},
        {K::StringLiteral, R"("Hi\n")"},
        {K::Delimiter, ","},
        {K::CharacterLiteral, R"('\q')"},
        {K::Delimiter, ";"},
        {K::BlockComment, "/* block */"},
        {K::Keyword, "return", "", 5, 5},
        {K::Identifier, "x_1"},
        {K::Delimiter, ";"},
        {K::Delimiter, "}"}
    })) return 1;

    if (!check("keywords",
        "array boolean char else for function if integer print return string void while", {
        {K::Keyword, "array"},
        {K::Keyword, "boolean"},
        {K::Keyword, "char"},
        {K::Keyword, "else"},
        {K::Keyword, "for"},
        {K::Keyword, "function"},
        {K::Keyword, "if"},
        {K::Keyword, "integer"},
        {K::Keyword, "print"},
        {K::Keyword, "return"},
        {K::Keyword, "string"},
        {K::Keyword, "void"},
        {K::Keyword, "while"}
    })) return 1;

    if (!check("operators and literals",
        R"(++ -- <= >= == != && || + - * / % ^ < > ! = ( ) [ ] { } ; , : true false iffy _id2 007 "" "a\"b" 'A' '\n' '\0' '\'' '\\' "\q")", {
        {K::Operator, "++"}, {K::Operator, "--"},
        {K::Operator, "<="}, {K::Operator, ">="},
        {K::Operator, "=="}, {K::Operator, "!="},
        {K::Operator, "&&"}, {K::Operator, "||"},
        {K::Operator, "+"}, {K::Operator, "-"},
        {K::Operator, "*"}, {K::Operator, "/"},
        {K::Operator, "%"}, {K::Operator, "^"},
        {K::Operator, "<"}, {K::Operator, ">"},
        {K::Operator, "!"}, {K::Operator, "="},

        {K::Delimiter, "("}, {K::Delimiter, ")"},
        {K::Delimiter, "["}, {K::Delimiter, "]"},
        {K::Delimiter, "{"}, {K::Delimiter, "}"},
        {K::Delimiter, ";"}, {K::Delimiter, ","},
        {K::Delimiter, ":"},

        {K::BooleanLiteral, "true"},
        {K::BooleanLiteral, "false"},
        {K::Identifier, "iffy"},
        {K::Identifier, "_id2"},
        {K::IntegerLiteral, "007"},
        {K::StringLiteral, R"("")"},
        {K::StringLiteral, R"("a\"b")"},
        {K::CharacterLiteral, "'A'"},
        {K::CharacterLiteral, R"('\n')"},
        {K::CharacterLiteral, R"('\0')"},
        {K::CharacterLiteral, R"('\'')"},
        {K::CharacterLiteral, R"('\\')"},
        {K::StringLiteral, R"("\q")"}
    })) return 1;

    if (!check("errors and recovery",
        "@ & |\r\n'' x\n\"oops\nz\n/* open", {
        {K::Invalid, "@", "Unexpected character", 1, 1},
        {K::Invalid, "&", "Unexpected character", 1, 3},
        {K::Invalid, "|", "Unexpected character", 1, 5},
        {K::Invalid, "''",
         "Character literal must contain exactly one character", 2, 1},
        {K::Identifier, "x", "", 2, 4},
        {K::Invalid, "\"oops",
         "Unterminated string literal", 3, 1},
        {K::Identifier, "z", "", 4, 1},
        {K::Invalid, "/* open",
         "Unterminated block comment", 5, 1}
    })) return 1;

    const std::string id256(256, 'a');
    const std::string id257(257, 'a');

    const std::string str256 =
        "\"" + std::string(256, 'b') + "\"";

    const std::string str257 =
        "\"" + std::string(257, 'b') + "\"";

    if (!check("length limits",
        id256 + " " + id257 + " " + str256 + " " + str257, {
        {K::Identifier, id256},
        {K::Invalid, id257,
         "Identifier exceeds 256 characters"},
        {K::StringLiteral, str256},
        {K::Invalid, str257,
         "String literal exceeds 256 characters"}
    })) return 1;

    std::cout << "All Lexer tests passed!\n";
    return 0;
}
