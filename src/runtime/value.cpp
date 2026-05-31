#include "velo/runtime/value.h"

namespace Velo::Runtime {
    auto cloneValue(const Value &value) -> Value {
        if (std::holds_alternative<StructValuePtr>(value)) {
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

        if (std::holds_alternative<ArrayValuePtr>(value)) {
            const auto &arrayValue = std::get<ArrayValuePtr>(value);
            if (arrayValue == nullptr) {
                return ArrayValuePtr {};
            }

            auto cloned = std::make_shared<ArrayValue>();
            cloned->elements.reserve(arrayValue->elements.size());

            for (const auto &element : arrayValue->elements) {
                cloned->elements.push_back(cloneValue(element));
            }

            return cloned;
        }

        return value;
    }
}
