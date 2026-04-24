#ifndef INC_VELO_RUNTIME_RUNTIME_H
#define INC_VELO_RUNTIME_RUNTIME_H

#include "builtin_registry.h"

namespace Velo::Runtime {
    class Runtime final {
    public:
        Runtime();

        [[nodiscard]] auto builtins() const -> const BuiltinRegistry&;
        [[nodiscard]] auto builtins() -> BuiltinRegistry&;

    private:
        void registerStdConsole();

        BuiltinRegistry _registry {};
    };
}

#endif //INC_VELO_RUNTIME_RUNTIME_H
