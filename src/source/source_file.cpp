#include <algorithm>
#include <velo/source/source_file.h>

namespace Velo::Source {
    SourceFile::SourceFile(std::string path, std::string content) : _path(std::move(path)), _content(std::move(content)) {
        buildLineOffset();
    }

    auto SourceFile::path() const -> const std::string& {
        return _path;
    }

    auto SourceFile::content() const -> const std::string& {
        return _content;
    }

    auto SourceFile::size() const -> std::size_t {
        return _content.size();
    }

    auto SourceFile::empty() const -> bool {
        return _content.empty();
    }

    auto SourceFile::locationAt(std::size_t offset) const -> std::optional<SourceLocation> {
        if (offset > _content.size()) {
            return std::nullopt;
        }

        const auto it = std::ranges::upper_bound(_lineOffsets.begin(), _lineOffsets.end(), offset);
        const auto lineIdx = it == _lineOffsets.begin() ? std::size_t {0} : static_cast<std::size_t>(std::distance(_lineOffsets.begin(), it) - 1);
        const auto lineStartOffset = _lineOffsets[lineIdx];

        return SourceLocation {
            offset,
            lineIdx + 1,
            (offset - lineStartOffset) + 1
        };
    }

    auto SourceFile::range(std::size_t beginOffset, std::size_t endOffset) const -> std::optional<SourceRange> {
        if (beginOffset > endOffset) {
            return std::nullopt;
        }

        const auto beginLocation = locationAt(beginOffset);
        const auto endLocation = locationAt(endOffset);

        if (!beginLocation.has_value() || !endLocation.has_value()) {
            return std::nullopt;
        }

        return SourceRange {
            *beginLocation,
            *endLocation
        };
    }

    void SourceFile::buildLineOffset() {
        _lineOffsets.clear();
        _lineOffsets.push_back(0);

        for (std::size_t idx = 0; idx < _content.size(); ++idx) {
            if (_content[idx] == '\n') {
                _lineOffsets.push_back(idx + 1);
            }
        }
    }
}
