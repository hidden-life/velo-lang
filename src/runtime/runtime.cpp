#include "velo/runtime/runtime.h"

#include <iostream>
#include <limits>
#include <sstream>
#include <optional>
#include <cctype>
#include <charconv>

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

        auto decodeStringEscapes(const std::string &value) -> std::string {
            std::string result;
            result.reserve(value.size());

            for (std::size_t idx = 0U; idx < value.size(); ++idx) {
                const char ch = value[idx];

                if (ch != '\\' || idx + 1U >= value.size()) {
                    result.push_back(ch);
                    continue;
                }

                const char escaped = value[idx + 1U];
                ++idx;

                switch (escaped) {
                    case '"':
                        result.push_back('"');
                        break;

                    case '\\':
                        result.push_back('\\');
                        break;

                    case '/':
                        result.push_back('/');
                        break;

                    case 'b':
                        result.push_back('\b');
                        break;

                    case 'f':
                        result.push_back('\f');
                        break;

                    case 'n':
                        result.push_back('\n');
                        break;

                    case 'r':
                        result.push_back('\r');
                        break;

                    case 't':
                        result.push_back('\t');
                        break;

                    default:
                        // Keep unknown escapes as-is for now.
                        result.push_back('\\');
                        result.push_back(escaped);
                        break;
                }
            }

            return result;
        }

        auto jsonValueToString(const JsonValuePtr &value) -> std::optional<std::string> {
            if (value == nullptr) {
                return std::nullopt;
            }

            switch (value->kind) {
                case JsonValueKind::Null:
                    return std::string("null");

                case JsonValueKind::Bool:
                    return value->boolValue ? std::string("true") : std::string("false");

                case JsonValueKind::Int:
                    return std::to_string(value->intValue);

                case JsonValueKind::String:
                    return "\"" + escapeJsonString(value->stringValue) + "\"";

                case JsonValueKind::Array: {
                    std::ostringstream stream;
                    stream << "[";

                    for (std::size_t idx = 0U; idx < value->arrayValues.size(); ++idx) {
                        if (idx > 0U) {
                            stream << ",";
                        }

                        const auto elementJson = jsonValueToString(value->arrayValues[idx]);
                        if (!elementJson.has_value()) {
                            return std::nullopt;
                        }

                        stream << *elementJson;
                    }

                    stream << "]";

                    return stream.str();
                }

                case JsonValueKind::Object: {
                    std::ostringstream stream;
                    stream << "{";

                    std::size_t idx = 0U;
                    for (const auto &[key, entryValue]: value->objectValues) {
                        if (idx > 0U) {
                            stream << ",";
                        }

                        const auto entryJson = jsonValueToString(entryValue);
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
            }

            return std::nullopt;
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

            if (std::holds_alternative<JsonValuePtr>(value)) {
                return jsonValueToString(std::get<JsonValuePtr>(value));
            }

            return std::nullopt;
        }

        class JsonParser final {
        public:
            explicit JsonParser(std::string_view text): _text(text) {}

            [[nodiscard]] auto parse() -> std::optional<JsonValuePtr> {
                skipWhitespace();

                auto value = parseValue();
                if (!value.has_value()) {
                    return std::nullopt;
                }

                skipWhitespace();

                if (!isAtEnd()) {
                    return std::nullopt;
                }

                return value;
            }

        private:
            [[nodiscard]] auto parseValue() -> std::optional<JsonValuePtr> {
                skipWhitespace();
                if (isAtEnd()) {
                    return std::nullopt;
                }

                const char ch = peek();
                if (ch == '"') {
                    return parseStringValue();
                }

                if (ch == '-' || std::isdigit(static_cast<unsigned char>(ch)) != 0) {
                    return parseIntValue();
                }

                if (matchLiteral("true")) {
                    auto value = std::make_shared<JsonValue>();
                    value->kind = JsonValueKind::Bool;
                    value->boolValue = true;

                    return value;
                }

                if (matchLiteral("false")) {
                    auto value = std::make_shared<JsonValue>();
                    value->kind = JsonValueKind::Bool;
                    value->boolValue = false;

                    return value;
                }

                if (matchLiteral("null")) {
                    auto value = std::make_shared<JsonValue>();
                    value->kind = JsonValueKind::Null;

                    return value;
                }

                if (ch == '[') {
                    return parseArrayValue();
                }

                if (ch == '{') {
                    return parseObjectValue();
                }

                return std::nullopt;
            }

            [[nodiscard]] auto parseStringValue() -> std::optional<JsonValuePtr> {
                auto stringValue = parseString();
                if (!stringValue.has_value()) {
                    return std::nullopt;
                }

                auto value = std::make_shared<JsonValue>();
                value->kind = JsonValueKind::String;
                value->stringValue = std::move(*stringValue);

                return value;
            }

            [[nodiscard]] auto parseString() -> std::optional<std::string> {
                if (!consume('"')) {
                    return std::nullopt;
                }

                std::string result;
                while (!isAtEnd()) {
                    const char ch = advance();
                    if (ch == '"') {
                        return result;
                    }

                    if (ch != '\\') {
                        result.push_back(ch);
                        continue;
                    }

                    if (isAtEnd()) {
                        return std::nullopt;
                    }

                    const char escaped = advance();
                    switch (escaped) {
                        case '"':
                            result.push_back('"');
                            break;
                        case '\\':
                            result.push_back('\\');
                            break;
                        case '/':
                            result.push_back('/');
                            break;
                        case 'b':
                            result.push_back('\b');
                            break;
                        case 'f':
                            result.push_back('\f');
                            break;
                        case 'n':
                            result.push_back('\n');
                            break;
                        case 'r':
                            result.push_back('\r');
                            break;
                        case 't':
                            result.push_back('\t');
                            break;
                        default:
                            return std::nullopt;
                    }
                }

                return std::nullopt;
            }

            [[nodiscard]] auto parseIntValue() -> std::optional<JsonValuePtr> {
                const auto start = _offset;
                if (peek() == '-') {
                    [[maybe_unused]] auto c = advance();
                }

                if (isAtEnd() || std::isdigit(static_cast<unsigned char>(peek())) == 0) {
                    return std::nullopt;
                }

                while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek())) != 0) {
                    [[maybe_unused]] auto c = advance();
                }

                const auto token = _text.substr(start, _offset - start);
                int number = 0;
                const auto *begin = token.data();
                const auto *end = token.data() + token.size();
                const auto parseResult = std::from_chars(begin, end, number);

                if (parseResult.ec != std::errc {} || parseResult.ptr != end) {
                    return std::nullopt;
                }

                auto value = std::make_shared<JsonValue>();
                value->kind = JsonValueKind::Int;
                value->intValue = number;

                return value;
            }

            [[nodiscard]] auto parseArrayValue() -> std::optional<JsonValuePtr> {
                if (!consume('[')) {
                    return std::nullopt;
                }

                auto value = std::make_shared<JsonValue>();
                value->kind = JsonValueKind::Array;

                skipWhitespace();

                if (consume(']')) {
                    return value;
                }

                while (true) {
                    auto element = parseValue();
                    if (!element.has_value()) {
                        return std::nullopt;
                    }

                    value->arrayValues.push_back(std::move(*element));

                    skipWhitespace();

                    if (consume(']')) {
                        break;
                    }

                    if (!consume(',')) {
                        return std::nullopt;
                    }
                }

                return value;
            }

            [[nodiscard]] auto parseObjectValue() -> std::optional<JsonValuePtr> {
                if (!consume('{')) {
                    return std::nullopt;
                }

                auto value = std::make_shared<JsonValue>();
                value->kind = JsonValueKind::Object;

                skipWhitespace();

                if (consume('}')) {
                    return value;
                }

                while (true) {
                    skipWhitespace();

                    auto key = parseString();
                    if (!key.has_value()) {
                        return std::nullopt;
                    }

                    skipWhitespace();

                    if (!consume(':')) {
                        return std::nullopt;
                    }

                    auto entryValue = parseValue();
                    if (!entryValue.has_value()) {
                        return std::nullopt;
                    }

                    value->objectValues[*key] = std::move(*entryValue);

                    skipWhitespace();

                    if (consume('}')) {
                        break;
                    }

                    if (!consume(',')) {
                        return std::nullopt;
                    }
                }

                return value;
            }

            void skipWhitespace() {
                while (!isAtEnd() && std::isspace(static_cast<unsigned char>(peek())) != 0) {
                    [[maybe_unused]] auto c = advance();
                }
            }

            [[nodiscard]] auto matchLiteral(std::string_view literal) -> bool {
                if (_text.substr(_offset, literal.size()) != literal) {
                    return false;
                }

                _offset += literal.size();

                return true;
            }

            [[nodiscard]] auto consume(char expected) -> bool {
                skipWhitespace();
                if (isAtEnd() || peek() != expected) {
                    return false;
                }

                [[maybe_unused]] auto c = advance();

                return true;
            }

            [[nodiscard]] auto peek() const -> char {
                return _text[_offset];
            }

            [[nodiscard]] auto advance() -> char {
                const char ch = _text[_offset];
                ++_offset;

                return ch;
            }

            [[nodiscard]] auto isAtEnd() const -> bool {
                return _offset >= _text.size();
            }

            std::string_view _text;
            std::size_t _offset { 0U };
        };
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
                {"json_serializable"},
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

        _registry.registerFunc(
            BuiltinFunction {
                "json::parse",
                {"string"},
                "json",
                [](const std::vector<Value> &args) -> ExecutionResult {
                    if (args.size() != 1U) {
                        return ExecutionResult {
                            .success = false,
                            .exitCode = 1,
                            .error = "json::parse expects exactly one argument."
                        };
                    }

                    if (!std::holds_alternative<std::string>(args.front())) {
                        return ExecutionResult {
                            .success = false,
                            .exitCode = 1,
                            .error = "json::parse expects a string argument."
                        };
                    }

                    const auto decoded = decodeStringEscapes(std::get<std::string>(args.front()));
                    JsonParser parser(decoded);
                    auto parsed = parser.parse();
                    if (!parsed.has_value()) {
                        return ExecutionResult {
                            .success = false,
                            .exitCode = 1,
                            .error = "Invalid JSON input."
                        };
                    }

                    return ExecutionResult {
                        .success = true,
                        .exitCode = 0,
                        .error = {},
                        .returnValue = *parsed
                    };
                }
            }
        );
    }
}
