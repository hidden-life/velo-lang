#include "velo/runtime/value.h"

namespace Velo::Runtime {
    auto cloneJsonValue(const JsonValuePtr &value) -> JsonValuePtr {
        if (value == nullptr) {
            return {};
        }

        auto cloned = std::make_shared<JsonValue>();
        cloned->kind = value->kind;
        cloned->boolValue = value->boolValue;
        cloned->intValue = value->intValue;
        cloned->stringValue = value->stringValue;

        cloned->arrayValues.reserve(value->arrayValues.size());
        for (const auto &element : value->arrayValues) {
            cloned->arrayValues.push_back(cloneJsonValue(element));
        }

        for (const auto &[key, entry]: value->objectValues) {
            cloned->objectValues.emplace(key, cloneJsonValue(entry));
        }

        return cloned;
    };

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

        if (std::holds_alternative<MapValuePtr>(value)) {
            const auto &mapValue = std::get<MapValuePtr>(value);
            if (mapValue == nullptr) {
                return MapValuePtr {};
            }

            auto cloned = std::make_shared<MapValue>();
            for (const auto &[key, entryValue] : mapValue->entries) {
                cloned->entries.emplace(key, cloneValue(entryValue));
            }

            return cloned;
        }

        if (std::holds_alternative<JsonValuePtr>(value)) {
            return cloneJsonValue(std::get<JsonValuePtr>(value));
        }

        if (std::holds_alternative<HttpRequestValuePtr>(value)) {
            const auto &requestValue = std::get<HttpRequestValuePtr>(value);
            if (requestValue == nullptr) {
                return HttpRequestValuePtr {};
            }

            auto cloned = std::make_shared<HttpRequestValue>();
            cloned->method = requestValue->method;
            cloned->path = requestValue->path;
            cloned->headers = requestValue->headers;
            cloned->body = requestValue->body;

            return cloned;
        }

        if (std::holds_alternative<HttpResponseValuePtr>(value)) {
            const auto &responseValue = std::get<HttpResponseValuePtr>(value);
            if (responseValue == nullptr) {
                return HttpResponseValuePtr {};
            }

            auto cloned = std::make_shared<HttpResponseValue>();
            cloned->status = responseValue->status;
            cloned->headers = responseValue->headers;
            cloned->body = responseValue->body;

            return cloned;
        }

        return value;
    }
}
