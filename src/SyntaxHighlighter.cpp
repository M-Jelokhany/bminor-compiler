#include "bminor/SyntaxHighlighter.h"

#include <sstream>
#include <unordered_map>

namespace bminor {
namespace {

using IdentifierStyles = std::unordered_map<std::size_t, std::string>;

void collectIdentifierStyles(const ASTNode* node, IdentifierStyles& styles) {
    if (node == nullptr) {
        return;
    }

    if (node->kind == NodeKind::VariableDeclaration) {
        styles[node->location.offset] = "variable";
    } else if (node->kind == NodeKind::Parameter) {
        styles[node->location.offset] = "parameter";
    } else if (node->kind == NodeKind::FunctionPrototype ||
               node->kind == NodeKind::FunctionDefinition) {
        styles[node->location.offset] = "function";
    } else if (node->kind == NodeKind::Identifier) {
        if (styles.find(node->location.offset) == styles.end()) {
            styles[node->location.offset] = "variable";
        }
    } else if (node->kind == NodeKind::CallExpression &&
               !node->children.empty() &&
               node->children[0]->kind == NodeKind::Identifier) {
        styles[node->children[0]->location.offset] = "function";
    }

    for (const auto& child : node->children) {
        collectIdentifierStyles(child.get(), styles);
    }
}

std::string htmlEscape(const std::string& text) {
    std::string result;
    for (char c : text) {
        switch (c) {
            case '&': result += "&amp;"; break;
            case '<': result += "&lt;"; break;
            case '>': result += "&gt;"; break;
            case '"': result += "&quot;"; break;
            default: result += c; break;
        }
    }
    return result;
}

std::string tokenClass(const Token& token, const IdentifierStyles& styles) {
    switch (token.kind) {
        case TokenKind::Keyword: return "kw";
        case TokenKind::Identifier: {
            auto found = styles.find(token.location.offset);
            if (found != styles.end()) return found->second;
            return "identifier";
        }
        case TokenKind::IntegerLiteral: return "number";
        case TokenKind::BooleanLiteral: return "boolean";
        case TokenKind::StringLiteral: return "string";
        case TokenKind::CharacterLiteral: return "character";
        case TokenKind::Operator: return "operator";
        case TokenKind::Delimiter: return "delimiter";
        case TokenKind::LineComment:
        case TokenKind::BlockComment: return "comment";
        case TokenKind::Invalid: return "invalid";
        case TokenKind::Whitespace:
        case TokenKind::EndOfFile: return "";
    }
    return "";
}

std::string ansiColor(const Token& token, const IdentifierStyles& styles) {
    switch (token.kind) {
        case TokenKind::Keyword: return "\033[1;35m";
        case TokenKind::Identifier: {
            auto found = styles.find(token.location.offset);
            if (found != styles.end()) {
                if (found->second == "function") return "\033[1;34m";
                if (found->second == "parameter") return "\033[1;36m";
                if (found->second == "variable") return "\033[1;32m";
            }
            return "\033[0;37m";
        }
        case TokenKind::IntegerLiteral:
        case TokenKind::BooleanLiteral: return "\033[1;33m";
        case TokenKind::StringLiteral:
        case TokenKind::CharacterLiteral: return "\033[0;32m";
        case TokenKind::Operator: return "\033[1;36m";
        case TokenKind::Delimiter: return "\033[0;37m";
        case TokenKind::LineComment:
        case TokenKind::BlockComment: return "\033[0;90m";
        case TokenKind::Invalid: return "\033[1;31m";
        case TokenKind::Whitespace:
        case TokenKind::EndOfFile: return "";
    }
    return "";
}

} // namespace

std::string SyntaxHighlighter::toHtml(const std::string& source,
                                      const std::vector<Token>& tokens,
                                      const ASTNode* ast) {
    IdentifierStyles styles;
    collectIdentifierStyles(ast, styles);

    std::ostringstream output;
    output << "<!DOCTYPE html>\n"
           << "<html lang=\"en\">\n"
           << "<head>\n"
           << "<meta charset=\"UTF-8\">\n"
           << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
           << "<title>B-Minor Syntax Highlighting</title>\n"
           << "<style>\n"
           << "body{margin:0;padding:24px;background:#f4f5f7;color:#20242b;"
              "font-family:Consolas,Monaco,monospace;}\n"
           << ".code{padding:22px;border-radius:10px;background:#171b24;color:#e6edf3;"
              "font-size:14px;line-height:1.65;overflow:auto;white-space:pre;}\n"
           << ".kw{color:#d2a8ff;font-weight:bold;}\n"
           << ".function{color:#79c0ff;font-weight:bold;}\n"
           << ".variable{color:#7ee787;}\n"
           << ".parameter{color:#56d4dd;}\n"
           << ".identifier{color:#e6edf3;}\n"
           << ".number,.boolean{color:#f2cc60;}\n"
           << ".string,.character{color:#a5d6ff;}\n"
           << ".operator{color:#ff7b72;}\n"
           << ".delimiter{color:#c9d1d9;}\n"
           << ".comment{color:#8b949e;font-style:italic;}\n"
           << ".invalid{color:#ffffff;background:#da3633;text-decoration:underline;}\n"
           << "</style>\n</head>\n<body>\n<pre class=\"code\">";

    std::size_t copiedUntil = 0;
    for (const Token& token : tokens) {
        if (token.kind == TokenKind::EndOfFile) break;

        const std::size_t start = token.location.offset;
        if (start > source.size()) continue;
        if (start < copiedUntil) continue;

        output << htmlEscape(source.substr(copiedUntil, start - copiedUntil));

        const std::string cssClass = tokenClass(token, styles);
        const std::string lexeme = source.substr(start, token.lexeme.size());
        if (cssClass.empty()) {
            output << htmlEscape(lexeme);
        } else {
            output << "<span class=\"" << cssClass << "\">"
                   << htmlEscape(lexeme) << "</span>";
        }

        copiedUntil = start + token.lexeme.size();
    }

    output << htmlEscape(source.substr(copiedUntil));
    output << "</pre>\n</body>\n</html>\n";
    return output.str();
}

std::string SyntaxHighlighter::toAnsi(const std::string& source,
                                      const std::vector<Token>& tokens,
                                      const ASTNode* ast) {
    IdentifierStyles styles;
    collectIdentifierStyles(ast, styles);

    std::ostringstream output;
    std::size_t copiedUntil = 0;

    for (const Token& token : tokens) {
        if (token.kind == TokenKind::EndOfFile) break;

        const std::size_t start = token.location.offset;
        if (start > source.size()) continue;
        if (start < copiedUntil) continue;

        output << source.substr(copiedUntil, start - copiedUntil);
        const std::string color = ansiColor(token, styles);
        const std::string lexeme = source.substr(start, token.lexeme.size());

        if (color.empty()) {
            output << lexeme;
        } else {
            output << color << lexeme << "\033[0m";
        }

        copiedUntil = start + token.lexeme.size();
    }

    output << source.substr(copiedUntil);
    return output.str();
}

} // namespace bminor
