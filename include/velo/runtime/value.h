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
}

#endif //INC_VELO_RUNTIME_VALUE_H
