#ifndef INC_VELO_MODULE_MODULE_SYMBOL_H
#define INC_VELO_MODULE_MODULE_SYMBOL_H

#include <string>
#include <unordered_map>

namespace Velo::Module {
    struct FunctionSymbol final {
        std::string name;
    };

    class ModuleSymbol final {
    public:
        explicit ModuleSymbol(std::string name);

        void addFunction(std::string funcName);

        [[nodiscard]] auto hasFunction(const std::string &name) const -> bool;
        [[nodiscard]] auto name() const -> const std::string&;

    private:
        std::string _name;
        std::unordered_map<std::string, FunctionSymbol> _functions {};
    };
}

#endif //INC_VELO_MODULE_MODULE_SYMBOL_H
