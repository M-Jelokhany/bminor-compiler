#pragma once
#include <cstddef>
#include <string>
#include "bminor/SourceLocation.h"

namespace bminor {

    class SourceCursor {
    public:
        SourceCursor(std::string source, std::string fileName);

        [[nodiscard]] bool atEnd() const;
        [[nodiscard]] char peek(std::size_t lookahead = 0) const;
        char advance();
        [[nodiscard]] SourceLocation location() const;

    private:
        std::string source_;
        std::string fileName_;

        std::size_t offset_ = 0;
        std::size_t line_ = 1;
        std::size_t column_ = 1;
    };

}