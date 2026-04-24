#include "velo/module/module_symbol.h"

#include <utility>

namespace Velo::Module {
    ModuleSymbol::ModuleSymbol(std::string name) : _name(std::move(name)) {
    }

    void ModuleSymbol::addFunction(std::string funcName, std::size_t arity) {
        const std::string key = funcName;

        _functions.insert_or_assign(key, FunctionSymbol{ funcName, arity });
    }

    auto ModuleSymbol::findFunction(const std::string &name) const -> const FunctionSymbol* {
        const auto it = _functions.find(name);
        if (it == _functions.end()) {
            return nullptr;
        }

        return &it->second;
    }

    auto ModuleSymbol::hasFunction(const std::string &name) const -> bool {
        return findFunction(name) != nullptr;
    }

    auto ModuleSymbol::name() const -> const std::string& {
        return _name;
    }
}
