#include "bminor/Parser.h"

#include <utility>

namespace bminor {
namespace {

bool isBasicType(const std::string& lexeme) {
    return lexeme == "integer" || lexeme == "boolean" ||
           lexeme == "char" || lexeme == "string";
}

std::unique_ptr<ASTNode> makeBinaryNode(
    const std::string& op,
    std::unique_ptr<ASTNode> left,
    std::unique_ptr<ASTNode> right
) {
    SourceLocation location;
    if (left) {
        location = left->location;
    }

    auto node = std::make_unique<ASTNode>(
        NodeKind::BinaryExpression, location, op
    );
    if (left) node->children.push_back(std::move(left));
    if (right) node->children.push_back(std::move(right));
    return node;
}

} // namespace

Parser::Parser(Lexer& lexer) : lexer_(lexer) {
    current_ = readToken();
    next_ = readToken();
}

Token Parser::readToken() {
    while (true) {
        Token token = lexer_.nextToken();

        if (token.kind == TokenKind::LineComment ||
            token.kind == TokenKind::BlockComment) {
            continue;
        }

        if (token.kind == TokenKind::Invalid) {
            errors_.push_back({token.location, token.errorMessage});
            continue;
        }

        return token;
    }
}

void Parser::advance() {
    current_ = next_;
    next_ = readToken();
}

bool Parser::isAtEnd() const {
    return current_.kind == TokenKind::EndOfFile;
}

bool Parser::check(const std::string& lexeme) const {
    return current_.lexeme == lexeme;
}

bool Parser::check(TokenKind kind) const {
    return current_.kind == kind;
}

bool Parser::checkNext(const std::string& lexeme) const {
    return next_.lexeme == lexeme;
}

bool Parser::match(const std::string& lexeme) {
    if (!check(lexeme)) return false;
    advance();
    return true;
}

bool Parser::match(TokenKind kind) {
    if (!check(kind)) return false;
    advance();
    return true;
}

bool Parser::consume(const std::string& lexeme,
                     const std::string& message) {
    if (check(lexeme)) {
        advance();
        return true;
    }
    reportError(message);
    return false;
}

bool Parser::consume(TokenKind kind,
                     const std::string& message) {
    if (check(kind)) {
        advance();
        return true;
    }
    reportError(message);
    return false;
}

void Parser::reportError(const std::string& message) {
    errors_.push_back({current_.location, message});
}

void Parser::synchronizeDeclaration() {
    while (!isAtEnd()) {
        if (match(";")) return;
        if (check(TokenKind::Identifier) && checkNext(":")) return;
        advance();
    }
}

void Parser::synchronizeStatement() {
    while (!isAtEnd() && !check("}")) {
        if (match(";")) return;
        if (check("return") || check("print") ||
            check("if") || check("for")) {
            return;
        }
        advance();
    }
}

std::unique_ptr<ASTNode> Parser::makeNode(
    NodeKind kind,
    const std::string& value
) {
    return std::make_unique<ASTNode>(kind, current_.location, value);
}

std::unique_ptr<ASTNode> Parser::parseProgram() {
    auto program = makeNode(NodeKind::Program);

    while (!isAtEnd()) {
        const std::size_t start = current_.location.offset;
        auto declaration = parseDeclaration();
        if (declaration) {
            program->children.push_back(std::move(declaration));
        }

        if (!isAtEnd() && current_.location.offset == start) {
            advance();
        }
    }

    return program;
}

std::unique_ptr<ASTNode> Parser::parseDeclaration() {
    if (!check(TokenKind::Identifier)) {
        reportError("Expected a declaration name");
        synchronizeDeclaration();
        return nullptr;
    }

    Token name = current_;
    advance();

    if (!consume(":", "Expected ':' after declaration name")) {
        synchronizeDeclaration();
        return nullptr;
    }

    if (match("function")) {
        return parseFunctionDeclaration(name);
    }

    return parseVariableDeclaration(name);
}

std::unique_ptr<ASTNode> Parser::parseVariableDeclaration(
    const Token& name
) {
    auto node = std::make_unique<ASTNode>(
        NodeKind::VariableDeclaration, name.location, name.lexeme
    );

    auto type = parseType(false);
    if (type) node->children.push_back(std::move(type));

    if (match("=")) {
        auto initializer = parseInitializer();
        if (initializer) node->children.push_back(std::move(initializer));
    }

    consume(";", "Expected ';' after variable declaration");
    return node;
}

std::unique_ptr<ASTNode> Parser::parseFunctionDeclaration(
    const Token& name
) {
    auto returnType = parseReturnType();

    consume("(", "Expected '(' after function return type");

    auto parameters = makeNode(NodeKind::ParameterList);
    if (!check(")")) {
        parameters = parseParameterList();
    }

    consume(")", "Expected ')' after parameters");

    if (match(";")) {
        auto function = std::make_unique<ASTNode>(
            NodeKind::FunctionPrototype, name.location, name.lexeme
        );
        if (returnType) function->children.push_back(std::move(returnType));
        if (parameters) function->children.push_back(std::move(parameters));
        return function;
    }

    consume("=", "Expected ';' or '=' after function declaration");
    auto body = parseBlock();

    auto function = std::make_unique<ASTNode>(
        NodeKind::FunctionDefinition, name.location, name.lexeme
    );
    if (returnType) function->children.push_back(std::move(returnType));
    if (parameters) function->children.push_back(std::move(parameters));
    if (body) function->children.push_back(std::move(body));
    return function;
}

std::unique_ptr<ASTNode> Parser::parseType(bool parameterType) {
    if (match("array")) {
        consume("[", "Expected '[' after array");

        std::string length;
        if (parameterType) {
            consume("]", "Array parameter type must use empty brackets");
        } else {
            if (check(TokenKind::IntegerLiteral)) {
                length = current_.lexeme;
                advance();
            } else {
                reportError("Expected array size");
            }
            consume("]", "Expected ']' after array size");
        }

        if (!isBasicType(current_.lexeme)) {
            reportError("Expected basic type after array type");
            return nullptr;
        }

        auto baseType = makeNode(NodeKind::BasicType, current_.lexeme);
        advance();

        auto arrayType = makeNode(NodeKind::ArrayType, length);
        arrayType->children.push_back(std::move(baseType));
        return arrayType;
    }

    if (isBasicType(current_.lexeme)) {
        auto type = makeNode(NodeKind::BasicType, current_.lexeme);
        advance();
        return type;
    }

    reportError("Expected a type");
    if (!isAtEnd() && !check(";") && !check(")") && !check("=")) {
        advance();
    }
    return nullptr;
}

std::unique_ptr<ASTNode> Parser::parseReturnType() {
    if (match("void")) {
        return makeNode(NodeKind::BasicType, "void");
    }
    return parseType(false);
}

std::unique_ptr<ASTNode> Parser::parseInitializer() {
    if (check("{")) return parseArrayInitializer();
    return parseExpression();
}

std::unique_ptr<ASTNode> Parser::parseArrayInitializer() {
    auto node = makeNode(NodeKind::ArrayInitializer);
    consume("{", "Expected '{' to start array initializer");

    if (!check("}")) {
        auto first = parseExpression();
        if (first) node->children.push_back(std::move(first));

        while (match(",")) {
            auto item = parseExpression();
            if (item) node->children.push_back(std::move(item));
        }
    } else {
        reportError("Array initializer cannot be empty");
    }

    consume("}", "Expected '}' after array initializer");
    return node;
}

std::unique_ptr<ASTNode> Parser::parseParameterList() {
    auto list = makeNode(NodeKind::ParameterList);

    auto parameter = parseParameter();
    if (parameter) list->children.push_back(std::move(parameter));

    while (match(",")) {
        parameter = parseParameter();
        if (parameter) list->children.push_back(std::move(parameter));
    }

    return list;
}

std::unique_ptr<ASTNode> Parser::parseParameter() {
    if (!check(TokenKind::Identifier)) {
        reportError("Expected parameter name");
        return nullptr;
    }

    Token name = current_;
    advance();
    consume(":", "Expected ':' after parameter name");

    auto parameter = std::make_unique<ASTNode>(
        NodeKind::Parameter, name.location, name.lexeme
    );
    auto type = parseType(true);
    if (type) parameter->children.push_back(std::move(type));
    return parameter;
}

std::unique_ptr<ASTNode> Parser::parseBlock() {
    auto block = makeNode(NodeKind::Block);
    consume("{", "Expected '{' to start block");

    while (!isAtEnd() && !check("}")) {
        const std::size_t start = current_.location.offset;
        auto item = parseBlockItem();
        if (item) block->children.push_back(std::move(item));

        if (!isAtEnd() && current_.location.offset == start) {
            synchronizeStatement();
            if (current_.location.offset == start && !isAtEnd() && !check("}")) {
                advance();
            }
        }
    }

    consume("}", "Expected '}' after block");
    return block;
}

std::unique_ptr<ASTNode> Parser::parseBlockItem() {
    if (check(TokenKind::Identifier) && checkNext(":")) {
        Token name = current_;
        advance();
        consume(":", "Expected ':' after variable name");
        if (check("function")) {
            reportError("Function declarations are not allowed inside a block");
            synchronizeDeclaration();
            return nullptr;
        }
        return parseVariableDeclaration(name);
    }

    return parseStatement();
}

std::unique_ptr<ASTNode> Parser::parseStatement() {
    if (check("{")) return parseBlock();
    if (match("return")) return parseReturnStatement();
    if (match("print")) return parsePrintStatement();
    if (match("if")) return parseIfStatement();
    if (match("for")) return parseForStatement();
    return parseExpressionStatement();
}

std::unique_ptr<ASTNode> Parser::parseExpressionStatement() {
    auto node = makeNode(NodeKind::ExpressionStatement);
    auto expression = parseExpression();
    if (expression) node->children.push_back(std::move(expression));
    consume(";", "Expected ';' after expression");
    return node;
}

std::unique_ptr<ASTNode> Parser::parseReturnStatement() {
    auto node = makeNode(NodeKind::ReturnStatement);
    if (!check(";")) {
        auto expression = parseExpression();
        if (expression) node->children.push_back(std::move(expression));
    }
    consume(";", "Expected ';' after return statement");
    return node;
}

std::unique_ptr<ASTNode> Parser::parsePrintStatement() {
    auto node = makeNode(NodeKind::PrintStatement);
    auto expression = parseExpression();
    if (expression) node->children.push_back(std::move(expression));

    while (match(",")) {
        expression = parseExpression();
        if (expression) node->children.push_back(std::move(expression));
    }

    consume(";", "Expected ';' after print statement");
    return node;
}

std::unique_ptr<ASTNode> Parser::parseIfStatement() {
    auto node = makeNode(NodeKind::IfStatement);
    consume("(", "Expected '(' after if");

    auto condition = parseExpression();
    if (condition) node->children.push_back(std::move(condition));

    consume(")", "Expected ')' after if condition");

    auto thenBranch = parseStatement();
    if (thenBranch) node->children.push_back(std::move(thenBranch));

    if (match("else")) {
        auto elseBranch = parseStatement();
        if (elseBranch) node->children.push_back(std::move(elseBranch));
    }

    return node;
}

std::unique_ptr<ASTNode> Parser::parseForStatement() {
    auto node = makeNode(NodeKind::ForStatement);
    consume("(", "Expected '(' after for");

    if (!check(";")) {
        auto init = parseExpression();
        if (init) node->children.push_back(std::move(init));
    } else {
        node->children.push_back(makeNode(NodeKind::EmptyExpression));
    }
    consume(";", "Expected first ';' in for statement");

    if (!check(";")) {
        auto condition = parseExpression();
        if (condition) node->children.push_back(std::move(condition));
    } else {
        node->children.push_back(makeNode(NodeKind::EmptyExpression));
    }
    consume(";", "Expected second ';' in for statement");

    if (!check(")")) {
        auto update = parseExpression();
        if (update) node->children.push_back(std::move(update));
    } else {
        node->children.push_back(makeNode(NodeKind::EmptyExpression));
    }
    consume(")", "Expected ')' after for clauses");

    auto body = parseStatement();
    if (body) node->children.push_back(std::move(body));
    return node;
}

std::unique_ptr<ASTNode> Parser::parseExpression() {
    return parseAssignmentExpression();
}

std::unique_ptr<ASTNode> Parser::parseAssignmentExpression() {
    auto left = parseLogicalOrExpression();
    if (match("=")) {
        auto right = parseAssignmentExpression();
        return makeBinaryNode("=", std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parseLogicalOrExpression() {
    auto expression = parseLogicalAndExpression();
    while (match("||")) {
        auto right = parseLogicalAndExpression();
        expression = makeBinaryNode("||", std::move(expression), std::move(right));
    }
    return expression;
}

std::unique_ptr<ASTNode> Parser::parseLogicalAndExpression() {
    auto expression = parseEqualityExpression();
    while (match("&&")) {
        auto right = parseEqualityExpression();
        expression = makeBinaryNode("&&", std::move(expression), std::move(right));
    }
    return expression;
}

std::unique_ptr<ASTNode> Parser::parseEqualityExpression() {
    auto expression = parseRelationalExpression();
    while (check("==") || check("!=")) {
        std::string op = current_.lexeme;
        advance();
        auto right = parseRelationalExpression();
        expression = makeBinaryNode(op, std::move(expression), std::move(right));
    }
    return expression;
}

std::unique_ptr<ASTNode> Parser::parseRelationalExpression() {
    auto expression = parseAdditiveExpression();
    while (check("<") || check("<=") || check(">") || check(">=")) {
        std::string op = current_.lexeme;
        advance();
        auto right = parseAdditiveExpression();
        expression = makeBinaryNode(op, std::move(expression), std::move(right));
    }
    return expression;
}

std::unique_ptr<ASTNode> Parser::parseAdditiveExpression() {
    auto expression = parseMultiplicativeExpression();
    while (check("+") || check("-")) {
        std::string op = current_.lexeme;
        advance();
        auto right = parseMultiplicativeExpression();
        expression = makeBinaryNode(op, std::move(expression), std::move(right));
    }
    return expression;
}

std::unique_ptr<ASTNode> Parser::parseMultiplicativeExpression() {
    auto expression = parsePowerExpression();
    while (check("*") || check("/") || check("%")) {
        std::string op = current_.lexeme;
        advance();
        auto right = parsePowerExpression();
        expression = makeBinaryNode(op, std::move(expression), std::move(right));
    }
    return expression;
}

std::unique_ptr<ASTNode> Parser::parsePowerExpression() {
    auto left = parseUnaryExpression();
    if (match("^")) {
        auto right = parsePowerExpression();
        return makeBinaryNode("^", std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parseUnaryExpression() {
    if (check("-") || check("!")) {
        std::string op = current_.lexeme;
        SourceLocation location = current_.location;
        advance();

        auto node = std::make_unique<ASTNode>(
            NodeKind::UnaryExpression, location, op
        );
        auto operand = parseUnaryExpression();
        if (operand) node->children.push_back(std::move(operand));
        return node;
    }

    return parsePostfixExpression();
}

std::unique_ptr<ASTNode> Parser::parsePostfixExpression() {
    auto expression = parseAccessExpression();
    if (check("++") || check("--")) {
        std::string op = current_.lexeme;
        auto node = makeNode(NodeKind::PostfixExpression, op);
        if (expression) node->children.push_back(std::move(expression));
        advance();
        return node;
    }
    return expression;
}

std::unique_ptr<ASTNode> Parser::parseAccessExpression() {
    auto expression = parsePrimaryExpression();

    while (check("[") || check("(")) {
        if (match("[")) {
            auto index = parseExpression();
            consume("]", "Expected ']' after array index");

            auto node = makeNode(NodeKind::IndexExpression);
            if (expression) node->children.push_back(std::move(expression));
            if (index) node->children.push_back(std::move(index));
            expression = std::move(node);
        } else if (match("(")) {
            auto arguments = makeNode(NodeKind::ArgumentList);
            if (!check(")")) arguments = parseArgumentList();
            consume(")", "Expected ')' after function arguments");

            auto node = makeNode(NodeKind::CallExpression);
            if (expression) node->children.push_back(std::move(expression));
            if (arguments) node->children.push_back(std::move(arguments));
            expression = std::move(node);
        }
    }

    return expression;
}

std::unique_ptr<ASTNode> Parser::parsePrimaryExpression() {
    if (check(TokenKind::Identifier)) {
        auto node = makeNode(NodeKind::Identifier, current_.lexeme);
        advance();
        return node;
    }

    if (check(TokenKind::IntegerLiteral)) {
        auto node = makeNode(NodeKind::IntegerLiteral, current_.lexeme);
        advance();
        return node;
    }

    if (check(TokenKind::BooleanLiteral)) {
        auto node = makeNode(NodeKind::BooleanLiteral, current_.lexeme);
        advance();
        return node;
    }

    if (check(TokenKind::StringLiteral)) {
        auto node = makeNode(NodeKind::StringLiteral, current_.lexeme);
        advance();
        return node;
    }

    if (check(TokenKind::CharacterLiteral)) {
        auto node = makeNode(NodeKind::CharacterLiteral, current_.lexeme);
        advance();
        return node;
    }

    if (match("(")) {
        auto node = makeNode(NodeKind::ParenthesizedExpression);
        auto expression = parseExpression();
        if (expression) node->children.push_back(std::move(expression));
        consume(")", "Expected ')' after expression");
        return node;
    }

    reportError("Expected expression");
    auto node = makeNode(NodeKind::EmptyExpression, "error");

    if (!isAtEnd() && !check(";") && !check(")") &&
        !check("]") && !check("}") && !check(",")) {
        advance();
    }

    return node;
}

std::unique_ptr<ASTNode> Parser::parseArgumentList() {
    auto list = makeNode(NodeKind::ArgumentList);

    auto argument = parseExpression();
    if (argument) list->children.push_back(std::move(argument));

    while (match(",")) {
        argument = parseExpression();
        if (argument) list->children.push_back(std::move(argument));
    }

    return list;
}

const std::vector<ParseError>& Parser::errors() const {
    return errors_;
}

} // namespace bminor
