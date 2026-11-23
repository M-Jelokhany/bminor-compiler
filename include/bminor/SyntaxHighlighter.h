#pragma once

#include <string>
#include <vector>

#include "bminor/AST.h"
#include "bminor/Token.h"

namespace bminor {

class SyntaxHighlighter {
public:
    // Both functions keep all original spaces, tabs and newlines.
    static std::string toHtml(const std::string& source,
                              const std::vector<Token>& tokens,
                              const ASTNode* ast);

    static std::string toAnsi(const std::string& source,
                              const std::vector<Token>& tokens,
                              const ASTNode* ast);
};

} // namespace bminor
