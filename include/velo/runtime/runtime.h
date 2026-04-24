#ifndef INC_VELO_RUNTIME_RUNTIME_H
#define INC_VELO_RUNTIME_RUNTIME_H

#include "builtin_registry.h"
#include "velo/module/module_registry.h"

namespace Velo::Runtime {
    class Runtime final {
    public:
        Runtime();

        [[nodiscard]] auto builtins() const -> const BuiltinRegistry&;
        [[nodiscard]] auto builtins() -> BuiltinRegistry&;

        [[nodiscard]] auto modules() const -> const Module::ModuleRegistry&;

        void buildModulesFromBuiltins();

    private:
        void registerStdConsole();

        BuiltinRegistry _registry {};
        Module::ModuleRegistry _modules {};
    };
}

#endif //INC_VELO_RUNTIME_RUNTIME_H
