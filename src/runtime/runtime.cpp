#include "velo/runtime/runtime.h"

#include <iostream>

namespace Velo::Runtime {
    namespace {
        auto valueToString(const Value &value) -> std::string {
            if (std::holds_alternative<int>(value)) {
                return std::to_string(std::get<int>(value));
            }

            return std::get<std::string>(value);
        }
    }

    Runtime::Runtime() {
        registerStdConsole();
    }

    auto Runtime::builtins() const -> const BuiltinRegistry& {
        return _registry;
    }

    auto Runtime::builtins() -> BuiltinRegistry& {
        return _registry;
    }

    void Runtime::registerStdConsole() {
        _registry.registerFunc(
            BuiltinFunction {
                "console::println",
                1U,
                [](const std::vector<Value> &args) -> ExecutionResult {
                    if (args.size() != 1U) {
                        return ExecutionResult {
                            .success = false,
                            .exitCode = 1,
                            .error = "console::println expects exactly one argument."
                        };
                    }

                    std::cout << valueToString(args.front()) << std::endl;

                    return ExecutionResult {};
                }
            }
        );
    }
}
