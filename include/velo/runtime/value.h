#ifndef INC_VELO_RUNTIME_VALUE_H
#define INC_VELO_RUNTIME_VALUE_H

#include <variant>
#include <string>
#include <map>
#include <memory>
#include <vector>

namespace Velo::Runtime {
    enum class JsonValueKind {
        Null,
        Bool,
        Int,
        String,
        Array,
        Object,
    };

    struct StructValue;
    struct ArrayValue;
    struct MapValue;
    struct JsonValue;

    using StructValuePtr = std::shared_ptr<StructValue>;
    using ArrayValuePtr = std::shared_ptr<ArrayValue>;
    using MapValuePtr = std::shared_ptr<MapValue>;
    using JsonValuePtr = std::shared_ptr<JsonValue>;
    // Runtime value used by interpreter.
    using Value = std::variant<int, std::string, bool, StructValuePtr, ArrayValuePtr, MapValuePtr, JsonValuePtr>;

    struct StructValue final {
        std::string typeName {};
        std::map<std::string, Value> fields {};
    };

    struct ArrayValue final {
        std::vector<Value> elements {};
    };

    struct MapValue final {
        std::map<std::string, Value> entries {};
    };

    struct JsonValue final {
        JsonValueKind kind { JsonValueKind::Null };

        bool boolValue {false};
        int intValue {0};
        std::string stringValue {};
        std::vector<JsonValuePtr> arrayValues {};
        std::map<std::string, JsonValuePtr> objectValues {};
    };

    // Creates an independent runtime value.
    //
    // Primitive values are copied directly.
    // Struct values are deep-copied recursively so future field mutation does not
    // accidentally alias another local, parameter, return value, or struct field.
    [[nodiscard]] auto cloneValue(const Value &value) -> Value;
}

#endif //INC_VELO_RUNTIME_VALUE_H
