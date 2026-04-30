#include "velo/runtime/builtin_function.h"
#include <utility>

namespace Velo::Runtime {
    BuiltinFunction::BuiltinFunction(std::string name, std::size_t arity, std::string returnType, BuiltinHandler handler) :
        _name(std::move(name)), _arity(arity), _returnType(std::move(returnType)), _handler(std::move(handler)) {
    }

    auto BuiltinFunction::name() const -> const std::string& {
        return _name;
    }

    auto BuiltinFunction::arity() const -> std::size_t {
        return _arity;
    }

    auto BuiltinFunction::call(const std::vector<Value> &arguments) const -> ExecutionResult {
        return _handler(arguments);
    }

    auto BuiltinFunction::moduleName() const -> std::string {
        const auto pos = _name.find("::");
        if (pos == std::string::npos) {
            return {};
        }

        return _name.substr(0, pos);
    }

    auto BuiltinFunction::functionName() const -> std::string {
        const auto pos = _name.find("::");
        if (pos == std::string::npos) {
            return _name;
        }

        return _name.substr(pos + 2U);
    }

    auto BuiltinFunction::returnType() const -> const std::string& {
        return _returnType;
    }
}
