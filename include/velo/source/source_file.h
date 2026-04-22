#ifndef INC_VELO_SOURCE_SOURCE_FILE_H
#define INC_VELO_SOURCE_SOURCE_FILE_H

#include <string>
#include <optional>
#include <vector>

#include "source_location.h"
#include "source_range.h"

namespace Velo::Source {
    // In-memory representation of a single source file
    class SourceFile final {
    public:
        SourceFile(std::string path, std::string content);

        [[nodiscard]] auto path() const -> const std::string&;
        [[nodiscard]] auto content() const -> const std::string&;
        [[nodiscard]] auto size() const -> std::size_t;
        [[nodiscard]] auto empty() const -> bool;

        [[nodiscard]] auto locationAt(std::size_t offset) const -> std::optional<SourceLocation>;
        [[nodiscard]] auto range(std::size_t beginOffset, std::size_t endOffset) const -> std::optional<SourceRange>;

    private:
        void buildLineOffset();

        std::string _path;
        std::string _content;
        std::vector<std::size_t> _lineOffsets;
    };
}

#endif //INC_VELO_SOURCE_SOURCE_FILE_H
