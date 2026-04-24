#include "velo/module/module_symbol.h"

#include <utility>

namespace Velo::Module {
    ModuleSymbol::ModuleSymbol(std::string name) : _name(std::move(name)) {
    }

    void ModuleSymbol::addFunction(std::string funcName) {
        _functions.emplace(funcName, FunctionSymbol{ funcName });
    }

    auto ModuleSymbol::hasFunction(const std::string &name) const -> bool {
        return _functions.contains(name);
    }

    auto ModuleSymbol::name() const -> const std::string& {
        return _name;
    }
}
