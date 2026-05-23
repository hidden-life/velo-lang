#ifndef INC_VELO_RUNTIME_BUILTIN_FUNCTION_H
#define INC_VELO_RUNTIME_BUILTIN_FUNCTION_H

#include <functional>
#include <string>
#include <vector>

#include "execution_result.h"
#include "value.h"

namespace Velo::Runtime {
    using BuiltinHandler = std::function<ExecutionResult(const std::vector<Value>&)>;

    class BuiltinFunction final {
    public:
        BuiltinFunction(
            std::string name,
            std::vector<std::string> parameterTypes,
            std::string returnType,
            BuiltinHandler handler
        );

        [[nodiscard]] auto name() const -> const std::string&;
        [[nodiscard]] auto arity() const -> std::size_t;
        [[nodiscard]] auto returnType() const -> const std::string&;
        [[nodiscard]] auto call(const std::vector<Value> &arguments) const -> ExecutionResult;

        [[nodiscard]] auto moduleName() const -> std::string;
        [[nodiscard]] auto functionName() const -> std::string;

        [[nodiscard]] auto parameterTypes() const -> const std::vector<std::string>&;

    private:
        std::string _name;
        std::size_t _arity {0};
        std::string _returnType;
        BuiltinHandler _handler;
        std::vector<std::string> _parameterTypes;
    };
}

#endif //INC_VELO_RUNTIME_BUILTIN_FUNCTION_H
