#pragma once

#include <string>
#include "bminor/SourceCursor.h"
#include "bminor/Token.h"

namespace bminor {

    class Lexer {
    public:
        Lexer(std::string source, std::string fileName);

        [[nodiscard]] Token nextToken();

    private:
        SourceCursor cursor_;
    };

}
