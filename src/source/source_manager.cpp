#include <velo/source/source_manager.h>

#include <filesystem>
#include <fstream>
#include <sstream>

namespace Velo::Source {
    auto SourceManager::addVirtualFile(std::string path, std::string content) -> const SourceFile& {
        _files.push_back(std::make_unique<SourceFile>(std::move(path), std::move(content)));

        return *_files.back();
    }

    auto SourceManager::loadFromDisk(const std::string &path) -> const SourceFile* {
        std::ifstream input(path, std::ios::in | std::ios::binary);
        if (!input.is_open()) {
            return nullptr;
        }

        std::ostringstream stream;
        stream << input.rdbuf();

        const auto normalizedPath = std::filesystem::path(path).lexically_normal().string();

        return &addVirtualFile(normalizedPath, stream.str());
    }

    auto SourceManager::files() const -> const std::vector<std::unique_ptr<SourceFile>>& {
        return _files;
    }
}
