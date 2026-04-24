#include "velo/runtime/builtin_function.h"
#include <utility>

namespace Velo::Runtime {
    BuiltinFunction::BuiltinFunction(std::string name, std::size_t arity, BuiltinHandler handler) :
        _name(std::move(name)), _arity(arity), _handler(std::move(handler)) {
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
}
