#ifndef SHANXUEXI_JSONHELPER_H
#define SHANXUEXI_JSONHELPER_H

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <cctype>
#include <cmath>

namespace shanxuexi {

/**
 * @brief 轻量级零依赖 C++17 JSON 解析与序列化器
 * 支持 Null, Bool, Number(double/int64), String, Array, Object
 * 用于本地持久化及兼容原版 Web 端的 JSON 备份文件导入/导出
 */
class JsonValue {
public:
    enum class Type {
        Null,
        Bool,
        Number,
        String,
        Array,
        Object
    };

    using ArrayType = std::vector<JsonValue>;
    using ObjectType = std::map<std::string, JsonValue>;

private:
    Type m_type = Type::Null;
    bool m_bool = false;
    double m_number = 0.0;
    std::string m_string;
    ArrayType m_array;
    ObjectType m_object;

public:
    JsonValue() : m_type(Type::Null) {}
    JsonValue(std::nullptr_t) : m_type(Type::Null) {}
    JsonValue(bool b) : m_type(Type::Bool), m_bool(b) {}
    JsonValue(int n) : m_type(Type::Number), m_number(static_cast<double>(n)) {}
    JsonValue(int64_t n) : m_type(Type::Number), m_number(static_cast<double>(n)) {}
    JsonValue(double n) : m_type(Type::Number), m_number(n) {}
    JsonValue(const char* s) : m_type(Type::String), m_string(s ? s : "") {}
    JsonValue(std::string s) : m_type(Type::String), m_string(std::move(s)) {}
    JsonValue(ArrayType arr) : m_type(Type::Array), m_array(std::move(arr)) {}
    JsonValue(ObjectType obj) : m_type(Type::Object), m_object(std::move(obj)) {}

    static JsonValue makeArray() { return JsonValue(ArrayType{}); }
    static JsonValue makeObject() { return JsonValue(ObjectType{}); }

    Type type() const { return m_type; }
    bool isNull() const { return m_type == Type::Null; }
    bool isBool() const { return m_type == Type::Bool; }
    bool isNumber() const { return m_type == Type::Number; }
    bool isString() const { return m_type == Type::String; }
    bool isArray() const { return m_type == Type::Array; }
    bool isObject() const { return m_type == Type::Object; }

    bool asBool(bool defaultVal = false) const {
        return m_type == Type::Bool ? m_bool : defaultVal;
    }
    double asDouble(double defaultVal = 0.0) const {
        return m_type == Type::Number ? m_number : defaultVal;
    }
    int asInt(int defaultVal = 0) const {
        return m_type == Type::Number ? static_cast<int>(m_number) : defaultVal;
    }
    int64_t asInt64(int64_t defaultVal = 0) const {
        return m_type == Type::Number ? static_cast<int64_t>(m_number) : defaultVal;
    }
    std::string asString(const std::string& defaultVal = "") const {
        return m_type == Type::String ? m_string : defaultVal;
    }
    const ArrayType& asArray() const { return m_array; }
    ArrayType& asArray() { return m_array; }
    const ObjectType& asObject() const { return m_object; }
    ObjectType& asObject() { return m_object; }

    void push_back(const JsonValue& v) {
        if (m_type != Type::Array) {
            m_type = Type::Array;
            m_array.clear();
        }
        m_array.push_back(v);
    }

    JsonValue& operator[](const std::string& key) {
        if (m_type != Type::Object) {
            m_type = Type::Object;
            m_object.clear();
        }
        return m_object[key];
    }

    const JsonValue& get(const std::string& key) const {
        static const JsonValue nullVal;
        if (m_type != Type::Object) return nullVal;
        auto it = m_object.find(key);
        return it != m_object.end() ? it->second : nullVal;
    }

    bool contains(const std::string& key) const {
        if (m_type != Type::Object) return false;
        return m_object.find(key) != m_object.end();
    }

    // 序列化为 JSON 字符串
    std::string dump(int indent = -1, int currentDepth = 0) const {
        std::ostringstream oss;
        std::string nl = (indent >= 0) ? "\n" : "";
        std::string pad = (indent >= 0) ? std::string(currentDepth * indent, ' ') : "";
        std::string innerPad = (indent >= 0) ? std::string((currentDepth + 1) * indent, ' ') : "";
        std::string sep = (indent >= 0) ? ": " : ":";

        switch (m_type) {
        case Type::Null:
            return "null";
        case Type::Bool:
            return m_bool ? "true" : "false";
        case Type::Number: {
            if (std::floor(m_number) == m_number && std::abs(m_number) < 9e15) {
                oss << static_cast<int64_t>(m_number);
            } else {
                oss << m_number;
            }
            return oss.str();
        }
        case Type::String:
            return escapeString(m_string);
        case Type::Array: {
            if (m_array.empty()) return "[]";
            oss << "[" << nl;
            for (size_t i = 0; i < m_array.size(); ++i) {
                oss << innerPad << m_array[i].dump(indent, currentDepth + 1);
                if (i + 1 < m_array.size()) oss << ",";
                oss << nl;
            }
            oss << pad << "]";
            return oss.str();
        }
        case Type::Object: {
            if (m_object.empty()) return "{}";
            oss << "{" << nl;
            size_t i = 0;
            for (const auto& kv : m_object) {
                oss << innerPad << escapeString(kv.first) << sep
                    << kv.second.dump(indent, currentDepth + 1);
                if (++i < m_object.size()) oss << ",";
                oss << nl;
            }
            oss << pad << "}";
            return oss.str();
        }
        }
        return "null";
    }

    // 从字符串解析 JSON
    static JsonValue parse(const std::string& src) {
        size_t pos = 0;
        JsonValue val = parseValue(src, pos);
        skipWhitespace(src, pos);
        return val;
    }

private:
    static std::string escapeString(const std::string& s) {
        std::ostringstream oss;
        oss << '"';
        for (unsigned char c : s) {
            switch (c) {
            case '"':  oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\b': oss << "\\b";  break;
            case '\f': oss << "\\f";  break;
            case '\n': oss << "\\n";  break;
            case '\r': oss << "\\r";  break;
            case '\t': oss << "\\t";  break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    oss << buf;
                } else {
                    oss << static_cast<char>(c);
                }
            }
        }
        oss << '"';
        return oss.str();
    }

    static void skipWhitespace(const std::string& s, size_t& pos) {
        // 跳过 UTF-8 BOM (如果存在)
        if (pos == 0 && s.size() >= 3 &&
            static_cast<unsigned char>(s[0]) == 0xEF &&
            static_cast<unsigned char>(s[1]) == 0xBB &&
            static_cast<unsigned char>(s[2]) == 0xBF) {
            pos = 3;
        }
        while (pos < s.size() && (s[pos] == ' ' || s[pos] == '\t' || s[pos] == '\n' || s[pos] == '\r')) {
            ++pos;
        }
    }

    static JsonValue parseValue(const std::string& s, size_t& pos) {
        skipWhitespace(s, pos);
        if (pos >= s.size()) {
            throw std::runtime_error("JSON 意外结束");
        }
        char c = s[pos];
        if (c == '{') return parseObject(s, pos);
        if (c == '[') return parseArray(s, pos);
        if (c == '"') return JsonValue(parseString(s, pos));
        if (c == 't' || c == 'f') return parseBool(s, pos);
        if (c == 'n') return parseNull(s, pos);
        if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) return parseNumber(s, pos);
        throw std::runtime_error(std::string("非法 JSON 字符: ") + c);
    }

    static JsonValue parseObject(const std::string& s, size_t& pos) {
        ObjectType obj;
        ++pos; // skip '{'
        skipWhitespace(s, pos);
        if (pos < s.size() && s[pos] == '}') {
            ++pos;
            return JsonValue(std::move(obj));
        }
        while (pos < s.size()) {
            skipWhitespace(s, pos);
            if (pos >= s.size() || s[pos] != '"') {
                throw std::runtime_error("JSON 对象键名必须是字符串");
            }
            std::string key = parseString(s, pos);
            skipWhitespace(s, pos);
            if (pos >= s.size() || s[pos] != ':') {
                throw std::runtime_error("JSON 对象缺少冒号 ':'");
            }
            ++pos; // skip ':'
            obj[key] = parseValue(s, pos);
            skipWhitespace(s, pos);
            if (pos < s.size() && s[pos] == ',') {
                ++pos;
                continue;
            }
            if (pos < s.size() && s[pos] == '}') {
                ++pos;
                return JsonValue(std::move(obj));
            }
            throw std::runtime_error("JSON 对象未正常闭合");
        }
        throw std::runtime_error("JSON 对象未闭合");
    }

    static JsonValue parseArray(const std::string& s, size_t& pos) {
        ArrayType arr;
        ++pos; // skip '['
        skipWhitespace(s, pos);
        if (pos < s.size() && s[pos] == ']') {
            ++pos;
            return JsonValue(std::move(arr));
        }
        while (pos < s.size()) {
            arr.push_back(parseValue(s, pos));
            skipWhitespace(s, pos);
            if (pos < s.size() && s[pos] == ',') {
                ++pos;
                continue;
            }
            if (pos < s.size() && s[pos] == ']') {
                ++pos;
                return JsonValue(std::move(arr));
            }
            throw std::runtime_error("JSON 数组未正常闭合");
        }
        throw std::runtime_error("JSON 数组未闭合");
    }

    static std::string parseString(const std::string& s, size_t& pos) {
        ++pos; // skip '"'
        std::string out;
        while (pos < s.size()) {
            char c = s[pos++];
            if (c == '"') return out;
            if (c == '\\') {
                if (pos >= s.size()) throw std::runtime_error("JSON 字符串转义未完成");
                char esc = s[pos++];
                switch (esc) {
                case '"':  out.push_back('"');  break;
                case '\\': out.push_back('\\'); break;
                case '/':  out.push_back('/');  break;
                case 'b':  out.push_back('\b'); break;
                case 'f':  out.push_back('\f'); break;
                case 'n':  out.push_back('\n'); break;
                case 'r':  out.push_back('\r'); break;
                case 't':  out.push_back('\t'); break;
                case 'u': {
                    if (pos + 4 > s.size()) throw std::runtime_error("非法 \\u 转义");
                    unsigned int cp = std::stoul(s.substr(pos, 4), nullptr, 16);
                    pos += 4;
                    if (cp <= 0x7F) {
                        out.push_back(static_cast<char>(cp));
                    } else if (cp <= 0x7FF) {
                        out.push_back(static_cast<char>(0xC0 | ((cp >> 6) & 0x1F)));
                        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
                    } else {
                        out.push_back(static_cast<char>(0xE0 | ((cp >> 12) & 0x0F)));
                        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
                        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
                    }
                    break;
                }
                default:
                    out.push_back(esc);
                    break;
                }
            } else {
                out.push_back(c);
            }
        }
        throw std::runtime_error("JSON 字符串缺少闭合引号");
    }

    static JsonValue parseBool(const std::string& s, size_t& pos) {
        if (s.compare(pos, 4, "true") == 0) {
            pos += 4;
            return JsonValue(true);
        }
        if (s.compare(pos, 5, "false") == 0) {
            pos += 5;
            return JsonValue(false);
        }
        throw std::runtime_error("非法布尔字面量");
    }

    static JsonValue parseNull(const std::string& s, size_t& pos) {
        if (s.compare(pos, 4, "null") == 0) {
            pos += 4;
            return JsonValue(nullptr);
        }
        throw std::runtime_error("非法 null 字面量");
    }

    static JsonValue parseNumber(const std::string& s, size_t& pos) {
        size_t start = pos;
        if (s[pos] == '-') ++pos;
        while (pos < s.size() && std::isdigit(static_cast<unsigned char>(s[pos]))) ++pos;
        if (pos < s.size() && s[pos] == '.') {
            ++pos;
            while (pos < s.size() && std::isdigit(static_cast<unsigned char>(s[pos]))) ++pos;
        }
        if (pos < s.size() && (s[pos] == 'e' || s[pos] == 'E')) {
            ++pos;
            if (pos < s.size() && (s[pos] == '+' || s[pos] == '-')) ++pos;
            while (pos < s.size() && std::isdigit(static_cast<unsigned char>(s[pos]))) ++pos;
        }
        double val = std::stod(s.substr(start, pos - start));
        return JsonValue(val);
    }
};

} // namespace shanxuexi

#endif // SHANXUEXI_JSONHELPER_H
