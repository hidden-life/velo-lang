#ifndef INC_VELO_MODULE_MODULE_REGISTRY_H
#define INC_VELO_MODULE_MODULE_REGISTRY_H

#include <string>

#include "module_symbol.h"

namespace Velo::Module {
    class ModuleRegistry final {
    public:
        void registerModule(ModuleSymbol module);

        [[nodiscard]] auto find(const std::string &name) const -> const ModuleSymbol*;
        // Returns mutable pointer to a module.
        [[nodiscard]] auto findMutable(const std::string &name) -> ModuleSymbol*;

    private:
        std::unordered_map<std::string, ModuleSymbol> _modules {};
    };
}

#endif //INC_VELO_MODULE_MODULE_REGISTRY_H
