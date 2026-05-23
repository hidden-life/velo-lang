#ifndef INC_VELO_RUNTIME_VALUE_H
#define INC_VELO_RUNTIME_VALUE_H

#include <variant>
#include <string>
#include <map>
#include <memory>

namespace Velo::Runtime {
    struct StructValue;

    using StructValuePtr = std::shared_ptr<StructValue>;
    // Runtime value used by interpreter.
    using Value = std::variant<int, std::string, bool, StructValuePtr>;

    struct StructValue final {
        std::string typeName {};
        std::map<std::string, Value> fields {};
    };

    // Creates an independent runtime value.
    //
    // Primitive values are copied directly.
    // Struct values are deep-copied recursively so future field mutation does not
    // accidentally alias another local, parameter, return value, or struct field.
    [[nodiscard]] auto cloneValue(const Value &value) -> Value;
}

#endif //INC_VELO_RUNTIME_VALUE_H
