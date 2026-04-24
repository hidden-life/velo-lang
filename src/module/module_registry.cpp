#include "velo/module/module_registry.h"

namespace Velo::Module {
    void ModuleRegistry::registerModule(ModuleSymbol module) {
        const std::string name = module.name();
        _modules.insert_or_assign(name, std::move(module));
    }

    auto ModuleRegistry::find(const std::string &name) const -> const ModuleSymbol* {
        const auto it = _modules.find(name);
        if (it == _modules.end()) {
            return nullptr;
        }

        return &it->second;
    }
}
