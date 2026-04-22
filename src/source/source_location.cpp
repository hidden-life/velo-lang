#include <velo/source/source_location.h>

namespace Velo::Source {
    SourceLocation::SourceLocation(std::size_t offset, std::size_t line, std::size_t column) : _offset(offset), _line(line), _column(column) {
    }

    auto SourceLocation::offset() const -> std::size_t {
        return _offset;
    }

    auto SourceLocation::line() const -> std::size_t {
        return _line;
    }

    auto SourceLocation::column() const -> std::size_t {
        return _column;
    }

    auto SourceLocation::isValid() const -> bool {
        return _line > 0 && _column > 0;
    }
}
