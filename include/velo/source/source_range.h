#ifndef INC_VELO_SOURCE_SOURCE_RANGE_H
#define INC_VELO_SOURCE_SOURCE_RANGE_H

#include "source_location.h"

namespace Velo::Source {
    // A range inside a source file from begin to end
    class SourceRange {
    public:
        SourceRange() = default;
        SourceRange(SourceLocation begin, SourceLocation end);

        [[nodiscard]] auto begin() const -> const SourceLocation&;
        [[nodiscard]] auto end() const -> const SourceLocation&;

        [[nodiscard]] auto isValid() const -> bool;

    private:
        SourceLocation _begin {};
        SourceLocation _end {};
    };
}

#endif //INC_VELO_SOURCE_SOURCE_RANGE_H
