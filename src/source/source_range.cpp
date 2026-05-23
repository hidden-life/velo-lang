#include <velo/source/source_range.h>

namespace Velo::Source {
    SourceRange::SourceRange(SourceLocation begin, SourceLocation end) : _begin(begin), _end(end) {
    }

    auto SourceRange::begin() const -> const SourceLocation& {
        return _begin;
    }

    auto SourceRange::end() const -> const SourceLocation& {
        return _end;
    }

    auto SourceRange::isValid() const -> bool {
        return _begin.isValid() && _end.isValid() && _begin <= _end;
    }
}
