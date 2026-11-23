
#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "bminor/SourceLocation.h"

namespace bminor {

enum class NodeKind {
    Program,
    VariableDeclaration,
    FunctionPrototype,
    FunctionDefinition,
    ParameterList,
    Parameter,
    BasicType,
    ArrayType,
    ArrayInitializer,
    Block,
    ExpressionStatement,
    ReturnStatement,
    PrintStatement,
    IfStatement,
    ForStatement,
    EmptyExpression,
    BinaryExpression,
    UnaryExpression,
    PostfixExpression,
    CallExpression,
    ArgumentList,
    IndexExpression,
    ParenthesizedExpression,
    Identifier,
    IntegerLiteral,
    BooleanLiteral,
    StringLiteral,
    CharacterLiteral
};

struct ASTNode {
    NodeKind kind;
    SourceLocation location;
    std::string value;

    std::string typeAnnotation;

    std::vector<std::unique_ptr<ASTNode>> children;

    ASTNode(NodeKind nodeKind, SourceLocation nodeLocation,
            const std::string& nodeValue = "")
        : kind(nodeKind), location(std::move(nodeLocation)), value(nodeValue) {
    }
};

std::string nodeKindName(NodeKind kind);
void printAst(const ASTNode* node, int indentation = 0);

} // namespace bminor
