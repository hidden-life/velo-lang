#ifndef INC_VELO_RUNTIME_BUILTIN_REGISTRY_H
#define INC_VELO_RUNTIME_BUILTIN_REGISTRY_H

#include "builtin_function.h"

namespace Velo::Runtime {
    class BuiltinRegistry final {
    public:
        void registerFunc(BuiltinFunction func);

        [[nodiscard]] auto find(const std::string &name) const -> const BuiltinFunction*;
        [[nodiscard]] auto contains(const std::string &name) const -> bool;

        [[nodiscard]] auto all() const -> const std::unordered_map<std::string, BuiltinFunction>&;

    private:
        std::unordered_map<std::string, BuiltinFunction> _functions {};
    };
}

#endif //INC_VELO_RUNTIME_BUILTIN_REGISTRY_H
