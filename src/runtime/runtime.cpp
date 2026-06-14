#include "velo/runtime/runtime.h"

#include <iostream>
#include <limits>
#include <sstream>
#include <optional>

namespace Velo::Runtime {
    namespace {
        auto valueToString(const Value &value) -> std::string {
            if (std::holds_alternative<int>(value)) {
                return std::to_string(std::get<int>(value));
            }

            if (std::holds_alternative<bool>(value)) {
                return std::get<bool>(value) ? "true" : "false";
            }

            if (std::holds_alternative<std::string>(value)) {
                return std::get<std::string>(value);
            }

            if (std::holds_alternative<StructValuePtr>(value)) {
                const auto &structVal = std::get<StructValuePtr>(value);
                if (structVal == nullptr) {
                    return "<struct:null>";
                }

                return "<struct " + structVal->typeName + ">";
            }

            if (std::holds_alternative<ArrayValuePtr>(value)) {
                const auto &arrayVal = std::get<ArrayValuePtr>(value);
                if (arrayVal == nullptr) {
                    return "<array:null>";
                }

                return "<array len=" + std::to_string(arrayVal->elements.size()) + ">";
            }

            if (std::holds_alternative<MapValuePtr>(value)) {
                const auto &mapVal = std::get<MapValuePtr>(value);
                if (mapVal == nullptr) {
                    return "<map:null>";
                }

                return "<map len=" + std::to_string(mapVal->entries.size()) + ">";
            }

            return "<unknown>";
        }

        auto escapeJsonString(const std::string &value) -> std::string {
            std::ostringstream stream;

            for (const char ch : value) {
                switch (ch) {
                    case '"':
                        stream << "\\\"";
                        break;
                    case '\\':
                        stream << "\\\\";
                        break;
                    case '\b':
                        stream << "\\b";
                        break;
                    case '\f':
                        stream << "\\f";
                        break;
                    case '\n':
                        stream << "\\n";
                        break;
                    case '\r':
                        stream << "\\r";
                        break;
                    case '\t':
                        stream << "\\t";
                        break;
                    default:
                        stream << ch;
                        break;
                }
            }

            return stream.str();
        }

        auto valueToJsonString(const Value &value) -> std::optional<std::string> {
            if (std::holds_alternative<int>(value)) {
                return std::to_string(std::get<int>(value));
            }

            if (std::holds_alternative<bool>(value)) {
                return std::get<bool>(value) ? std::string("true") : std::string("false");
            }

            if (std::holds_alternative<std::string>(value)) {
                return "\"" + escapeJsonString(std::get<std::string>(value)) + "\"";
            }

            if (std::holds_alternative<StructValuePtr>(value)) {
                const auto structValue = std::get<StructValuePtr>(value);
                if (structValue == nullptr) {
                    return std::nullopt;
                }

                std::ostringstream stream;
                stream << "{";

                std::size_t idx = 0U;
                for (const auto &[name, val] : structValue->fields) {
                    if (idx > 0U) {
                        stream << ",";
                    }

                    const auto fieldJson = valueToJsonString(val);
                    if (!fieldJson.has_value()) {
                        return std::nullopt;
                    }

                    stream << "\"" << escapeJsonString(name) << "\":";
                    stream << *fieldJson;

                    ++idx;
                }

                stream << "}";

                return stream.str();
            }

            if (std::holds_alternative<ArrayValuePtr>(value)) {
                const auto arrayValue = std::get<ArrayValuePtr>(value);
                if (arrayValue == nullptr) {
                    return std::nullopt;
                }

                std::ostringstream stream;
                stream << "[";

                for (std::size_t idx = 0U; idx < arrayValue->elements.size(); ++idx) {
                    if (idx > 0U) {
                        stream << ",";
                    }

                    const auto elementJson = valueToJsonString(arrayValue->elements[idx]);
                    if (!elementJson.has_value()) {
                        return std::nullopt;
                    }

                    stream << *elementJson;
                }

                stream << "]";

                return stream.str();
            }

            if (std::holds_alternative<MapValuePtr>(value)) {
                const auto mapValue = std::get<MapValuePtr>(value);
                if (mapValue == nullptr) {
                    return std::nullopt;
                }

                std::ostringstream stream;
                stream << "{";

                std::size_t idx = 0U;
                for (const auto &[key, val] : mapValue->entries) {
                    if (idx > 0U) {
                        stream << ",";
                    }

                    const auto entryJson = valueToJsonString(val);
                    if (!entryJson.has_value()) {
                        return std::nullopt;
                    }

                    stream << "\"" << escapeJsonString(key) << "\":";
                    stream << *entryJson;

                    ++idx;
                }

                stream << "}";

                return stream.str();
            }

            return std::nullopt;
        }
    }

    Runtime::Runtime() {
        registerStdConsole();
        registerStdString();
        registerStdInt();
        registerStdBool();

        // array
        registerStdArray();
        // map
        registerStdMap();
        // serialization
        registerStdJson();

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

    void Runtime::registerStdArray() {
        _registry.registerFunc(
            BuiltinFunction {
                "array::len",
                {"array"},
                "int",
                [](const std::vector<Value> &args) -> ExecutionResult {
                    if (args.size() != 1U) {
                        return ExecutionResult {
                            .success = false,
                            .exitCode = 1,
                            .error = "array::len expects exactly one argument."
                        };
                    }

                    if (!std::holds_alternative<ArrayValuePtr>(args[0])) {
                        return ExecutionResult {
                            .success = false,
                            .exitCode = 1,
                            .error = "array::len expects an array argument."
                        };
                    }

                    const auto arrayValue = std::get<ArrayValuePtr>(args[0]);
                    if (arrayValue == nullptr) {
                        return ExecutionResult {
                            .success = false,
                            .exitCode = 1,
                            .error = "array::len received a null array value."
                        };
                    }

                    return ExecutionResult {
                        .success = true,
                        .exitCode = 0,
                        .error = {},
                        .returnValue = static_cast<int>(arrayValue->elements.size())
                    };
                }
            }
        );
    }

    void Runtime::registerStdMap() {
        _registry.registerFunc(
            BuiltinFunction {
                "map::len",
                {"map"},
                "int",
                [](const std::vector<Value> &args) -> ExecutionResult {
                    if (args.size() != 1U) {
                        return ExecutionResult {
                            .success = false,
                            .exitCode = 1,
                            .error = "map::len expects exactly one argument."
                        };
                    }

                    if (!std::holds_alternative<MapValuePtr>(args[0])) {
                        return ExecutionResult {
                            .success = false,
                            .exitCode = 1,
                            .error = "map::len expects an map argument."
                        };
                    }

                    const auto mapValue = std::get<MapValuePtr>(args[0]);
                    if (mapValue == nullptr) {
                        return ExecutionResult {
                            .success = false,
                            .exitCode = 1,
                            .error = "map::len received a null map value."
                        };
                    }

                    return ExecutionResult {
                        .success = true,
                        .exitCode = 0,
                        .error = {},
                        .returnValue = static_cast<int>(mapValue->entries.size())
                    };
                }
            }
        );
    }

    void Runtime::registerStdJson() {
        _registry.registerFunc(
            BuiltinFunction {
                "json::stringify",
                {"json"},
                "string",
                [](const std::vector<Value> &args) -> ExecutionResult {
                    if (args.size() != 1U) {
                        return ExecutionResult {
                            .success = false,
                            .exitCode = 1,
                            .error = "json::stringify expects exactly one argument."
                        };
                    }

                    const auto jsonText = valueToJsonString(args.front());
                    if (!jsonText.has_value()) {
                        return ExecutionResult {
                            .success = false,
                            .exitCode = 1,
                            .error = "json::stringify does not support this value yet."
                        };
                    }

                    return ExecutionResult {
                        .success = true,
                        .exitCode = 0,
                        .error = {},
                        .returnValue = *jsonText
                    };
                }
            }
        );
    }
}
