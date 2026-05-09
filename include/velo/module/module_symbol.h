#ifndef INC_VELO_MODULE_MODULE_SYMBOL_H
#define INC_VELO_MODULE_MODULE_SYMBOL_H

#include <string>
#include <unordered_map>
#include <cstddef>
#include <vector>

namespace Velo::Module {
    struct FunctionSymbol final {
        std::string name;
        std::size_t arity {0};
        std::string returnType;

        // Declared parameter types used by semantic analysis.
        // Supported values for now: "int"m "string", "bool", "any"
        std::vector<std::string> parameterTypes {};
    };

    class ModuleSymbol final {
    public:
        explicit ModuleSymbol(std::string name);
        // Adds a module function with its expected argument count.
        void addFunction(
            std::string funcName,
            std::size_t arity,
            std::string returnType,
            std::vector<std::string> parameterTypes
            );

        [[nodiscard]] auto findFunction(const std::string &name) const -> const FunctionSymbol*;
        [[nodiscard]] auto hasFunction(const std::string &name) const -> bool;
        [[nodiscard]] auto name() const -> const std::string&;

    private:
        std::string _name;
        std::unordered_map<std::string, FunctionSymbol> _functions {};
    };
}

#endif //INC_VELO_MODULE_MODULE_SYMBOL_H
