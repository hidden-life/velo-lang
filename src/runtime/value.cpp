#include "velo/runtime/value.h"

namespace Velo::Runtime {
    auto cloneValue(const Value &value) -> Value {
        if (!std::holds_alternative<StructValuePtr>(value)) {
            return value;
        }

        const auto &structValue = std::get<StructValuePtr>(value);
        if (structValue == nullptr) {
            return StructValuePtr {};
        }

        auto cloned = std::make_shared<StructValue>();
        cloned->typeName = structValue->typeName;

        for (const auto &[fieldName, fieldValue] : structValue->fields) {
            cloned->fields.emplace(fieldName, cloneValue(fieldValue));
        }

        return cloned;
    }
}
