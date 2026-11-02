#include "json.h"

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>


namespace json {

    const Value* Value::find(const std::string& name) const {
        for (size_t i = 0; i < member_names.size(); i++) {
            if (member_names[i] == name) {
                return &member_values[i];
            }
        }
        return nullptr;
    }

    const Value& Value::require(const std::string& name) const {
        const Value* member = find(name);
        if (member == nullptr) {
            throw std::runtime_error("missing key in scene file: " + name);
        }
        return *member;
    }

    namespace {

        class Reader {
        public:
            explicit Reader(const std::string& source) : source(source), position(0) {}

            Value read_document() {
                Value root = read_value();
                skip_whitespace();
                if (!at_end()) {
                    fail("unexpected text after the end of the document");
                }
                return root;
            }

        private:
            const std::string& source;
            size_t position;

            [[noreturn]] void fail(const std::string& message) const {
                throw std::runtime_error("JSON error at character " + std::to_string(position) + ": " + message);
            }

            bool at_end() const {
                return position >= source.size();
            }

            char current() const {
                if (at_end()) {
                    return '\0';
                }
                return source[position];
            }

            static bool is_whitespace(char c) {
                return c == ' ' || c == '\t' || c == '\n' || c == '\r';
            }

            static bool ends_literal(char c) {
                return is_whitespace(c) || c == ',' || c == ':' || c == '}' || c == ']';
            }

            void skip_whitespace() {
                while (!at_end() && is_whitespace(current())) {
                    position++;
                }
            }

            void expect(char expected) {
                skip_whitespace();
                if (current() != expected) {
                    fail(std::string("expected '") + expected + "'");
                }
                position++;
            }

            Value read_value() {
                skip_whitespace();
                if (current() == '{') {
                    return read_object();
                }
                if (current() == '[') {
                    return read_array();
                }
                Value value;
                if (current() == '"') {
                    value.text = read_string();
                } else {
                    value.text = read_literal();
                }
                return value;
            }

            Value read_object() {
                Value object;
                object.kind = Kind::Object;
                expect('{');
                skip_whitespace();
                if (current() == '}') {
                    position++;
                    return object;
                }

                bool has_more_members = true;
                while (has_more_members) {
                    skip_whitespace();
                    if (current() != '"') {
                        fail("expected a key in double quotes");
                    }
                    std::string name = read_string();
                    expect(':');
                    Value member = read_value();
                    object.member_names.push_back(name);
                    object.member_values.push_back(std::move(member));

                    skip_whitespace();
                    if (current() == ',') {
                        position++;
                    } else {
                        expect('}');
                        has_more_members = false;
                    }
                }
                return object;
            }

            Value read_array() {
                Value array;
                array.kind = Kind::Array;
                expect('[');
                skip_whitespace();
                if (current() == ']') {
                    position++;
                    return array;
                }

                bool has_more_items = true;
                while (has_more_items) {
                    Value item = read_value();
                    array.array_items.push_back(std::move(item));

                    skip_whitespace();
                    if (current() == ',') {
                        position++;
                    } else {
                        expect(']');
                        has_more_items = false;
                    }
                }
                return array;
            }

            std::string read_literal() {
                size_t start = position;
                while (!at_end() && !ends_literal(current())) {
                    position++;
                }
                if (position == start) {
                    fail("expected a value");
                }
                return source.substr(start, position - start);
            }

            std::string read_string() {
                expect('"');
                std::string result;
                while (current() != '"') {
                    if (at_end()) {
                        fail("string is not closed");
                    }
                    if (current() == '\\') {
                        position++;
                        append_escaped_character(result);
                    } else {
                        result += current();
                        position++;
                    }
                }
                position++;
                return result;
            }

            void append_escaped_character(std::string& result) {
                char escaped = current();
                position++;
                switch (escaped) {
                    case '"':  result += '"';  break;
                    case '\\': result += '\\'; break;
                    case '/':  result += '/';  break;
                    case 'b':  result += '\b'; break;
                    case 'f':  result += '\f'; break;
                    case 'n':  result += '\n'; break;
                    case 'r':  result += '\r'; break;
                    case 't':  result += '\t'; break;
                    case 'u':  append_utf8(result, read_four_hex_digits()); break;
                    default:   fail("invalid escape sequence in string");
                }
            }

            unsigned read_four_hex_digits() {
                unsigned value = 0;
                for (int i = 0; i < 4; i++) {
                    char digit = current();
                    position++;
                    value *= 16;
                    if (digit >= '0' && digit <= '9') {
                        value += digit - '0';
                    } else if (digit >= 'a' && digit <= 'f') {
                        value += digit - 'a' + 10;
                    } else if (digit >= 'A' && digit <= 'F') {
                        value += digit - 'A' + 10;
                    } else {
                        fail("invalid \\u escape in string");
                    }
                }
                return value;
            }

            static void append_utf8(std::string& result, unsigned code_point) {
                if (code_point < 0x80) {
                    result += static_cast<char>(code_point);
                } else if (code_point < 0x800) {
                    result += static_cast<char>(0xC0 | (code_point >> 6));
                    result += static_cast<char>(0x80 | (code_point & 0x3F));
                } else {
                    result += static_cast<char>(0xE0 | (code_point >> 12));
                    result += static_cast<char>(0x80 | ((code_point >> 6) & 0x3F));
                    result += static_cast<char>(0x80 | (code_point & 0x3F));
                }
            }
        };
    }

    Value parse_document(const std::string& source) {
        Reader reader(source);
        return reader.read_document();
    }
}
