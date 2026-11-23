
#include "bminor/AST.h"

#include <iostream>

namespace bminor {

std::string nodeKindName(NodeKind kind) {
    switch (kind) {
        case NodeKind::Program: return "Program";
        case NodeKind::VariableDeclaration: return "VariableDeclaration";
        case NodeKind::FunctionPrototype: return "FunctionPrototype";
        case NodeKind::FunctionDefinition: return "FunctionDefinition";
        case NodeKind::ParameterList: return "ParameterList";
        case NodeKind::Parameter: return "Parameter";
        case NodeKind::BasicType: return "BasicType";
        case NodeKind::ArrayType: return "ArrayType";
        case NodeKind::ArrayInitializer: return "ArrayInitializer";
        case NodeKind::Block: return "Block";
        case NodeKind::ExpressionStatement: return "ExpressionStatement";
        case NodeKind::ReturnStatement: return "ReturnStatement";
        case NodeKind::PrintStatement: return "PrintStatement";
        case NodeKind::IfStatement: return "IfStatement";
        case NodeKind::ForStatement: return "ForStatement";
        case NodeKind::EmptyExpression: return "EmptyExpression";
        case NodeKind::BinaryExpression: return "BinaryExpression";
        case NodeKind::UnaryExpression: return "UnaryExpression";
        case NodeKind::PostfixExpression: return "PostfixExpression";
        case NodeKind::CallExpression: return "CallExpression";
        case NodeKind::ArgumentList: return "ArgumentList";
        case NodeKind::IndexExpression: return "IndexExpression";
        case NodeKind::ParenthesizedExpression: return "ParenthesizedExpression";
        case NodeKind::Identifier: return "Identifier";
        case NodeKind::IntegerLiteral: return "IntegerLiteral";
        case NodeKind::BooleanLiteral: return "BooleanLiteral";
        case NodeKind::StringLiteral: return "StringLiteral";
        case NodeKind::CharacterLiteral: return "CharacterLiteral";
    }

    return "Unknown";
}

void printAst(const ASTNode* node, int indentation) {
    if (node == nullptr) {
        return;
    }

    for (int i = 0; i < indentation; i++) {
        std::cout << "  ";
    }

    std::cout << nodeKindName(node->kind);

    if (!node->value.empty()) {
        std::cout << ": " << node->value;
    }

    if (!node->typeAnnotation.empty()) {
        std::cout << " [" << node->typeAnnotation << "]";
    }

    std::cout << std::endl;

    for (const auto& child : node->children) {
        printAst(child.get(), indentation + 1);
    }
}

} // namespace bminor
