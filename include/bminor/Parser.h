
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "bminor/AST.h"
#include "bminor/Lexer.h"

namespace bminor {

struct ParseError {
    SourceLocation location;
    std::string message;
};

class Parser {
public:
    explicit Parser(Lexer& lexer);

    std::unique_ptr<ASTNode> parseProgram();

    const std::vector<ParseError>& errors() const;

private:
    Lexer& lexer_;
    Token current_;
    Token next_;
    std::vector<ParseError> errors_;

    Token readToken();
    void advance();
    bool isAtEnd() const;

    bool check(const std::string& lexeme) const;
    bool check(TokenKind kind) const;
    bool checkNext(const std::string& lexeme) const;

    bool match(const std::string& lexeme);
    bool match(TokenKind kind);

    bool consume(const std::string& lexeme,
                 const std::string& message);

    bool consume(TokenKind kind,
                 const std::string& message);

    void reportError(const std::string& message);

    void synchronizeDeclaration();
    void synchronizeStatement();

    std::unique_ptr<ASTNode> makeNode(
        NodeKind kind,
        const std::string& value = ""
    );

    // Declarations
    std::unique_ptr<ASTNode> parseDeclaration();
    std::unique_ptr<ASTNode> parseVariableDeclaration(
        const Token& name
    );
    std::unique_ptr<ASTNode> parseFunctionDeclaration(
        const Token& name
    );
    std::unique_ptr<ASTNode> parseType(bool parameterType = false);
    std::unique_ptr<ASTNode> parseReturnType();
    std::unique_ptr<ASTNode> parseInitializer();
    std::unique_ptr<ASTNode> parseArrayInitializer();
    std::unique_ptr<ASTNode> parseParameterList();
    std::unique_ptr<ASTNode> parseParameter();

    // Statements
    std::unique_ptr<ASTNode> parseBlock();
    std::unique_ptr<ASTNode> parseBlockItem();
    std::unique_ptr<ASTNode> parseStatement();
    std::unique_ptr<ASTNode> parseExpressionStatement();
    std::unique_ptr<ASTNode> parseReturnStatement();
    std::unique_ptr<ASTNode> parsePrintStatement();
    std::unique_ptr<ASTNode> parseIfStatement();
    std::unique_ptr<ASTNode> parseForStatement();

    // Expressions
    std::unique_ptr<ASTNode> parseExpression();
    std::unique_ptr<ASTNode> parseAssignmentExpression();
    std::unique_ptr<ASTNode> parseLogicalOrExpression();
    std::unique_ptr<ASTNode> parseLogicalAndExpression();
    std::unique_ptr<ASTNode> parseEqualityExpression();
    std::unique_ptr<ASTNode> parseRelationalExpression();
    std::unique_ptr<ASTNode> parseAdditiveExpression();
    std::unique_ptr<ASTNode> parseMultiplicativeExpression();
    std::unique_ptr<ASTNode> parsePowerExpression();
    std::unique_ptr<ASTNode> parseUnaryExpression();
    std::unique_ptr<ASTNode> parsePostfixExpression();
    std::unique_ptr<ASTNode> parseAccessExpression();
    std::unique_ptr<ASTNode> parsePrimaryExpression();
    std::unique_ptr<ASTNode> parseArgumentList();
};

} // namespace bminor
