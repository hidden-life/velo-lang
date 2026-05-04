#ifndef INC_VELO_RUNTIME_VALUE_H
#define INC_VELO_RUNTIME_VALUE_H

#include <variant>
#include <string>

namespace Velo::Runtime {
    // Runtime value used by the first interpreter implementation.
    using Value = std::variant<int, std::string, bool>;
}

#endif //INC_VELO_RUNTIME_VALUE_H
