#include "bminor/Lexer.h"
#include "bminor/Parser.h"
#include "bminor/SyntaxHighlighter.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

bool readFile(const std::string& path, std::string& source) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;
    std::ostringstream contents;
    contents << file.rdbuf();
    source = contents.str();
    return true;
}

std::string tokenKindName(bminor::TokenKind kind) {
    using bminor::TokenKind;
    switch (kind) {
        case TokenKind::Keyword: return "Keyword";
        case TokenKind::Identifier: return "Identifier";
        case TokenKind::IntegerLiteral: return "IntegerLiteral";
        case TokenKind::BooleanLiteral: return "BooleanLiteral";
        case TokenKind::StringLiteral: return "StringLiteral";
        case TokenKind::CharacterLiteral: return "CharacterLiteral";
        case TokenKind::Operator: return "Operator";
        case TokenKind::Delimiter: return "Delimiter";
        case TokenKind::LineComment: return "LineComment";
        case TokenKind::BlockComment: return "BlockComment";
        case TokenKind::Whitespace: return "Whitespace";
        case TokenKind::Invalid: return "Invalid";
        case TokenKind::EndOfFile: return "EndOfFile";
    }
    return "Unknown";
}

std::vector<bminor::Token> collectTokens(const std::string& source,
                                         const std::string& fileName) {
    bminor::Lexer lexer(source, fileName);
    std::vector<bminor::Token> tokens;
    while (true) {
        bminor::Token token = lexer.nextToken();
        tokens.push_back(token);
        if (token.kind == bminor::TokenKind::EndOfFile) break;
    }
    return tokens;
}

void printTokens(const std::vector<bminor::Token>& tokens) {
    for (const auto& token : tokens) {
        std::cout << token.location.line << ':' << token.location.column
                  << "  " << tokenKindName(token.kind)
                  << "  `" << token.lexeme << '`';
        if (!token.errorMessage.empty()) {
            std::cout << "  ERROR: " << token.errorMessage;
        }
        std::cout << '\n';
    }
}

void printUsage(const char* programName) {
    std::cout << "B-Minor Compiler - Phase 1\n\n"
              << "Usage:\n  " << programName
              << " <source-file> [options]\n\n"
              << "Options:\n"
              << "  --tokens          Print the token stream\n"
              << "  --ast             Print the Abstract Syntax Tree\n"
              << "  --html <file>     Create an HTML syntax-highlighted file\n"
              << "  --ansi            Print syntax-highlighted source in the terminal\n"
              << "  --check           Only report whether parsing succeeded\n"
              << "  --help            Show this message\n\n"
              << "If no output option is selected, the AST is printed by default.\n";
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc < 2 || std::string(argv[1]) == "--help") {
        printUsage(argv[0]);
        return argc < 2 ? 2 : 0;
    }

    const std::string inputPath = argv[1];
    bool showTokens = false;
    bool showAst = false;
    bool showAnsi = false;
    bool checkOnly = false;
    std::string htmlPath;

    for (int i = 2; i < argc; ++i) {
        const std::string option = argv[i];
        if (option == "--tokens") showTokens = true;
        else if (option == "--ast") showAst = true;
        else if (option == "--ansi") showAnsi = true;
        else if (option == "--check") checkOnly = true;
        else if (option == "--html") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --html needs an output file name.\n";
                return 2;
            }
            htmlPath = argv[++i];
        } else if (option == "--help") {
            printUsage(argv[0]);
            return 0;
        } else {
            std::cerr << "Unknown option: " << option << '\n';
            printUsage(argv[0]);
            return 2;
        }
    }

    std::string source;
    if (!readFile(inputPath, source)) {
        std::cerr << "Could not open source file: " << inputPath << '\n';
        return 2;
    }

    const std::vector<bminor::Token> tokens = collectTokens(source, inputPath);
    bminor::Lexer parserLexer(source, inputPath);
    bminor::Parser parser(parserLexer);
    std::unique_ptr<bminor::ASTNode> ast = parser.parseProgram();

    if (showTokens) {
        std::cout << "=== TOKENS ===\n";
        printTokens(tokens);
    }

    if (showAst || (!showTokens && htmlPath.empty() && !showAnsi && !checkOnly)) {
        std::cout << "=== ABSTRACT SYNTAX TREE ===\n";
        bminor::printAst(ast.get());
    }

    for (const auto& error : parser.errors()) {
        std::cerr << error.location.file << ':'
                  << error.location.line << ':'
                  << error.location.column
                  << ": error: " << error.message << '\n';
    }

    if (!htmlPath.empty()) {
        std::ofstream htmlFile(htmlPath, std::ios::binary);
        if (!htmlFile) {
            std::cerr << "Could not create HTML file: " << htmlPath << '\n';
            return 2;
        }
        htmlFile << bminor::SyntaxHighlighter::toHtml(source, tokens, ast.get());
        std::cout << "HTML output written to: " << htmlPath << '\n';
    }

    if (showAnsi) {
        std::cout << bminor::SyntaxHighlighter::toAnsi(source, tokens, ast.get());
        if (source.empty() || source.back() != '\n') std::cout << '\n';
    }

    if (parser.errors().empty()) {
        std::cout << "Parsing completed successfully.\n";
        return 0;
    }

    std::cout << "Parsing completed with " << parser.errors().size()
              << " error(s).\n";
    return 1;
}
