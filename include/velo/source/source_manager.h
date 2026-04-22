#ifndef INC_VELO_SOURCE_SOURCE_MANAGER_H
#define INC_VELO_SOURCE_SOURCE_MANAGER_H

#include <string>
#include <memory>
#include <vector>

#include "source_file.h"

namespace Velo::Source {
    class SourceManager final {
    public:
        SourceManager() = default;

        [[nodiscard]] auto addVirtualFile(std::string path, std::string content) -> const SourceFile&;
        [[nodiscard]] auto loadFromDisk(const std::string &path) -> const SourceFile*;

        [[nodiscard]] auto files() const -> const std::vector<std::unique_ptr<SourceFile>>&;

    private:
        std::vector<std::unique_ptr<SourceFile>> _files {};
    };
}

#endif //INC_VELO_SOURCE_SOURCE_MANAGER_H
