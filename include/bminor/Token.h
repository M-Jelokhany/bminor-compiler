#pragma once

#include <string>
#include "bminor/SourceLocation.h"

namespace bminor {

    enum class TokenKind {
        Keyword,
        Identifier,
        IntegerLiteral,
        BooleanLiteral,
        StringLiteral,
        CharacterLiteral,
        Operator,
        Delimiter,
        LineComment,
        BlockComment,
        Whitespace,
        Invalid,
        EndOfFile
    };

    struct Token {
        TokenKind kind = TokenKind::Invalid;
        std::string lexeme;
        SourceLocation location;
        std::string errorMessage = "";
    };

}
