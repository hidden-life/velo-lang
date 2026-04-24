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
            const std::string moduleName = func.moduleName();
            const std::string functionName = func.functionName();

            if (moduleName.empty()) {
                continue;
            }

            auto *module = _modules.find(moduleName);
            if (module == nullptr) {
                Module::ModuleSymbol newModule(moduleName);
                newModule.addFunction(functionName);
                _modules.registerModule(std::move(newModule));
            } else {
                const_cast<Module::ModuleSymbol*>(module)->addFunction(functionName);
            }
        }
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
