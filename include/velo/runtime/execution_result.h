#ifndef INC_VELO_RUNTIME_EXECUTION_RESULT_H
#define INC_VELO_RUNTIME_EXECUTION_RESULT_H

#include <string>

namespace Velo::Runtime {
    struct ExecutionResult final {
        bool success {true};
        int exitCode {0};
        std::string error;
    };
}

#endif //INC_VELO_RUNTIME_EXECUTION_RESULT_H
