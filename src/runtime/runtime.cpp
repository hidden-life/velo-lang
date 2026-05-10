#include "velo/runtime/runtime.h"

#include <iostream>
#include <limits>

namespace Velo::Runtime {
    namespace {
        auto valueToString(const Value &value) -> std::string {
            if (std::holds_alternative<int>(value)) {
                return std::to_string(std::get<int>(value));
            }

            if (std::holds_alternative<bool>(value)) {
                return std::get<bool>(value) ? "true" : "false";
            }

            return std::get<std::string>(value);
        }
    }

    Runtime::Runtime() {
        registerStdConsole();
        registerStdString();
        registerStdInt();
        registerStdBool();

        buildModulesFromBuiltins();
    }

    auto Runtime::builtins() const -> const BuiltinRegistry& {
        return _registry;
    }

    auto Runtime::builtins() -> BuiltinRegistry& {
        return _registry;
    }

    auto Runtime::modules() const -> const Module::ModuleRegistry& {
        return _modules;
    }

    void Runtime::buildModulesFromBuiltins() {
        for (const auto &[name, func] : _registry.all()) {
            static_cast<void>(name);

            const std::string moduleName = func.moduleName();
            const std::string functionName = func.functionName();

            if (moduleName.empty()) {
                continue;
            }

            auto *module = _modules.findMutable(moduleName);
            if (module == nullptr) {
                Module::ModuleSymbol newModule(moduleName);
                newModule.addFunction(functionName, func.arity(), func.returnType(), func.parameterTypes());
                _modules.registerModule(std::move(newModule));
                continue;
            }

            module->addFunction(functionName, func.arity(), func.returnType(), func.parameterTypes());
        }
    }

    void Runtime::registerStdConsole() {
        _registry.registerFunc(
            BuiltinFunction {
                "console::println",
                {"any"},
                "void",
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

    void Runtime::registerStdString() {
        _registry.registerFunc(
            BuiltinFunction {
                "string::len",
                {"string"},
                "int",
                [](const std::vector<Value> &args) -> ExecutionResult {
                    if (args.size() != 1U) {
                        return ExecutionResult {
                            .success = false,
                            .exitCode = 1,
                            .error = "string::len expects exactly one argument."
                        };
                    }

                    const auto &value = args.front();
                    if (!std::holds_alternative<std::string>(value)) {
                        return ExecutionResult {
                            .success = false,
                            .exitCode = 1,
                            .error = "string::len expects a string argument."
                        };
                    }

                    const auto &text = std::get<std::string>(value);
                    if (text.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
                        return ExecutionResult {
                            .success = false,
                            .exitCode = 1,
                            .error = "string::len result is too large for int."
                        };
                    }

                    return ExecutionResult {
                        .success = true,
                        .exitCode = 0,
                        .error = {},
                        .returnValue = static_cast<int>(text.size())
                    };
                }
            }
        );
    }

    void Runtime::registerStdInt() {
        _registry.registerFunc(
            BuiltinFunction {
                "int::toString",
                {"int"},
                "string",
                [](const std::vector<Value> &args) -> ExecutionResult {
                    if (args.size() != 1U) {
                        return ExecutionResult {
                            .success = false,
                            .exitCode = 1,
                            .error = "int::toString expects exactly one argument."
                        };
                    }

                    const auto &value = args.front();
                    if (!std::holds_alternative<int>(value)) {
                        return ExecutionResult {
                            .success = false,
                            .exitCode = 1,
                            .error = "int::toString expects an int argument."
                        };
                    }

                    return ExecutionResult {
                        .success = true,
                        .exitCode = 0,
                        .error = {},
                        .returnValue = std::to_string(std::get<int>(value))
                    };
                }
            }
        );
    }

    void Runtime::registerStdBool() {
        _registry.registerFunc(
            BuiltinFunction {
                "bool::toString",
                {"bool"},
                "string",
                [](const std::vector<Value> &args) -> ExecutionResult {
                    if (args.size() != 1U) {
                        return ExecutionResult {
                            .success = false,
                            .exitCode = 1,
                            .error = "bool::toString expects exactly one argument."
                        };
                    }

                    const auto &value = args.front();
                    if (!std::holds_alternative<bool>(value)) {
                        return ExecutionResult {
                            .success = false,
                            .exitCode = 1,
                            .error = "bool::toString expects a bool argument."
                        };
                    }

                    return ExecutionResult {
                        .success = true,
                        .exitCode = 0,
                        .error = {},
                        .returnValue = std::get<bool>(value) ? std::string("true") : std::string("false")
                    };
                }
            }
        );
    }
}
