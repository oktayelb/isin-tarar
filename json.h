#pragma once

#include <string>
#include <vector>


namespace json {

    enum class Kind { Object, Array, Text };

    struct Value {
        Kind kind = Kind::Text;
        std::string text;
        std::vector<std::string> member_names;
        std::vector<Value> member_values;
        std::vector<Value> array_items;

        const Value* find(const std::string& name) const;
        const Value& require(const std::string& name) const;
    };

    Value parse_document(const std::string& source);
}
