#ifndef INC_VELO_RUNTIME_EXECUTION_RESULT_H
#define INC_VELO_RUNTIME_EXECUTION_RESULT_H

#include <string>
#include <optional>
#include "value.h"

namespace Velo::Runtime {
    struct ExecutionResult final {
        bool success {true};
        int exitCode {0};
        std::string error {};

        // Optional value produced by a function-like runtime operation.
        // Void builtins leave this empty.
        std::optional<Value> returnValue {};
    };
}

#endif //INC_VELO_RUNTIME_EXECUTION_RESULT_H
