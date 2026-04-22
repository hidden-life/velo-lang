#ifndef INC_VELO_SOURCE_SOURCE_LOCATION_H
#define INC_VELO_SOURCE_SOURCE_LOCATION_H

#include <cstddef>
#include <compare>

namespace Velo::Source {
    // A concrete position inside a source file
    class SourceLocation final {
    public:
        SourceLocation() = default;
        SourceLocation(std::size_t offset, std::size_t line, std::size_t column);

        [[nodiscard]] auto offset() const -> std::size_t;
        [[nodiscard]] auto line() const -> std::size_t;
        [[nodiscard]] auto column() const -> std::size_t;

        [[nodiscard]] auto isValid() const -> bool;

        auto operator<=>(const SourceLocation&) const = default;

    private:
        std::size_t _offset {0};
        std::size_t _line {0};
        std::size_t _column {0};
    };
}

#endif //INC_VELO_SOURCE_SOURCE_LOCATION_H
