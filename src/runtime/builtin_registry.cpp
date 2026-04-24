#include "velo/runtime/builtin_registry.h"

namespace Velo::Runtime {
    void BuiltinRegistry::registerFunc(BuiltinFunction func) {
        const std::string name = func.name();
        _functions.insert_or_assign(name, std::move(func));
    }

    auto BuiltinRegistry::find(const std::string &name) const -> const BuiltinFunction* {
        const auto it = _functions.find(name);
        if (it == _functions.end()) {
            return nullptr;
        }

        return &it->second;
    }

    auto BuiltinRegistry::contains(const std::string &name) const -> bool {
        return _functions.contains(name);
    }
}
