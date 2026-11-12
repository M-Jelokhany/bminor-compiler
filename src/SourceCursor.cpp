#include "bminor/SourceCursor.h"
#include <utility>

namespace bminor {

    SourceCursor::SourceCursor(
        std::string source,
        std::string fileName
    )
        : source_(std::move(source)),
          fileName_(std::move(fileName)) {
    }

    bool SourceCursor::atEnd() const {
        return offset_ >= source_.size();
    }

    char SourceCursor::peek(std::size_t lookahead) const {
        if (lookahead >= source_.size() - offset_) {
            return '\0';
        }

        return source_[offset_ + lookahead];
    }

    char SourceCursor::advance() {
        if (atEnd()) {
            return '\0';
        }

        char current = source_[offset_];
        ++offset_;

        if (current == '\r') {
            ++line_;
            column_ = 1;
        }
        else if (current == '\n') {
            if (offset_ < 2 || source_[offset_ - 2] != '\r') {
                ++line_;
            }

            column_ = 1;
        }
        else {
            ++column_;
        }

        return current;
    }

    SourceLocation SourceCursor::location() const {
        return {fileName_, line_, column_, offset_};
    }

}
