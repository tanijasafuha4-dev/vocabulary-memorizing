#ifndef SHANXUEXI_JSONHELPER_H
#define SHANXUEXI_JSONHELPER_H

#include <string>
#include <vector>
#include <map>
#include <variant>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <cctype>
#include <cmath>
#include <cstdio>

namespace shanxuexi {

/**
 * @brief 轻量级零依赖 C++17 JSON 解析与序列化器
 * 内部采用 std::variant 按需存储 Null, Bool, Number(double/int64), String, Array, Object，
 * 支持 UTF-16 代理对转义、递归深度保护与尾部冗余字符校验。
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
    using StorageVariant = std::variant<std::nullptr_t, bool, double, std::string, ArrayType, ObjectType>;

private:
    StorageVariant m_data;
    static constexpr int kMaxDepth = 128;

public:
    JsonValue() : m_data(nullptr) {}
    JsonValue(std::nullptr_t) : m_data(nullptr) {}
    JsonValue(bool b) : m_data(b) {}
    JsonValue(int n) : m_data(static_cast<double>(n)) {}
    JsonValue(int64_t n) : m_data(static_cast<double>(n)) {}
    JsonValue(double n) : m_data(n) {}
    JsonValue(const char* s) : m_data(std::string(s ? s : "")) {}
    JsonValue(std::string s) : m_data(std::move(s)) {}
    JsonValue(ArrayType arr) : m_data(std::move(arr)) {}
    JsonValue(ObjectType obj) : m_data(std::move(obj)) {}

    static JsonValue makeArray() { return JsonValue(ArrayType{}); }
    static JsonValue makeObject() { return JsonValue(ObjectType{}); }

    Type type() const {
        if (std::holds_alternative<std::nullptr_t>(m_data)) return Type::Null;
        if (std::holds_alternative<bool>(m_data)) return Type::Bool;
        if (std::holds_alternative<double>(m_data)) return Type::Number;
        if (std::holds_alternative<std::string>(m_data)) return Type::String;
        if (std::holds_alternative<ArrayType>(m_data)) return Type::Array;
        return Type::Object;
    }

    bool isNull() const { return std::holds_alternative<std::nullptr_t>(m_data); }
    bool isBool() const { return std::holds_alternative<bool>(m_data); }
    bool isNumber() const { return std::holds_alternative<double>(m_data); }
    bool isString() const { return std::holds_alternative<std::string>(m_data); }
    bool isArray() const { return std::holds_alternative<ArrayType>(m_data); }
    bool isObject() const { return std::holds_alternative<ObjectType>(m_data); }

    bool asBool(bool defaultVal = false) const {
        return isBool() ? std::get<bool>(m_data) : defaultVal;
    }
    double asDouble(double defaultVal = 0.0) const {
        return isNumber() ? std::get<double>(m_data) : defaultVal;
    }
    int asInt(int defaultVal = 0) const {
        return isNumber() ? static_cast<int>(std::get<double>(m_data)) : defaultVal;
    }
    int64_t asInt64(int64_t defaultVal = 0) const {
        return isNumber() ? static_cast<int64_t>(std::get<double>(m_data)) : defaultVal;
    }
    std::string asString(const std::string& defaultVal = "") const {
        return isString() ? std::get<std::string>(m_data) : defaultVal;
    }
    const ArrayType& asArray() const {
        static const ArrayType kEmpty;
        return isArray() ? std::get<ArrayType>(m_data) : kEmpty;
    }
    ArrayType& asArray() {
        if (!isArray()) m_data = ArrayType{};
        return std::get<ArrayType>(m_data);
    }
    const ObjectType& asObject() const {
        static const ObjectType kEmpty;
        return isObject() ? std::get<ObjectType>(m_data) : kEmpty;
    }
    ObjectType& asObject() {
        if (!isObject()) m_data = ObjectType{};
        return std::get<ObjectType>(m_data);
    }

    void push_back(const JsonValue& v) {
        asArray().push_back(v);
    }

    JsonValue& operator[](const std::string& key) {
        return asObject()[key];
    }

    const JsonValue& get(const std::string& key) const {
        static const JsonValue nullVal;
        if (!isObject()) return nullVal;
        const auto& obj = std::get<ObjectType>(m_data);
        auto it = obj.find(key);
        return it != obj.end() ? it->second : nullVal;
    }

    bool contains(const std::string& key) const {
        if (!isObject()) return false;
        const auto& obj = std::get<ObjectType>(m_data);
        return obj.find(key) != obj.end();
    }

    // 序列化为 JSON 字符串
    std::string dump(int indent = -1, int currentDepth = 0) const {
        std::ostringstream oss;
        std::string nl = (indent >= 0) ? "\n" : "";
        std::string pad = (indent >= 0) ? std::string(currentDepth * indent, ' ') : "";
        std::string innerPad = (indent >= 0) ? std::string((currentDepth + 1) * indent, ' ') : "";
        std::string sep = (indent >= 0) ? ": " : ":";

        switch (type()) {
        case Type::Null:
            return "null";
        case Type::Bool:
            return std::get<bool>(m_data) ? "true" : "false";
        case Type::Number: {
            double num = std::get<double>(m_data);
            if (std::floor(num) == num && std::abs(num) < 9e15) {
                oss << static_cast<int64_t>(num);
            } else {
                oss << num;
            }
            return oss.str();
        }
        case Type::String:
            return escapeString(std::get<std::string>(m_data));
        case Type::Array: {
            const auto& arr = std::get<ArrayType>(m_data);
            if (arr.empty()) return "[]";
            oss << "[" << nl;
            for (size_t i = 0; i < arr.size(); ++i) {
                oss << innerPad << arr[i].dump(indent, currentDepth + 1);
                if (i + 1 < arr.size()) oss << ",";
                oss << nl;
            }
            oss << pad << "]";
            return oss.str();
        }
        case Type::Object: {
            const auto& obj = std::get<ObjectType>(m_data);
            if (obj.empty()) return "{}";
            oss << "{" << nl;
            size_t i = 0;
            for (const auto& kv : obj) {
                oss << innerPad << escapeString(kv.first) << sep
                    << kv.second.dump(indent, currentDepth + 1);
                if (++i < obj.size()) oss << ",";
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
        JsonValue val = parseValue(src, pos, 0);
        skipWhitespace(src, pos);
        if (pos != src.size()) {
            throw std::runtime_error("JSON 尾部存在多余非法字符");
        }
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

    static JsonValue parseValue(const std::string& s, size_t& pos, int depth) {
        if (depth > kMaxDepth) {
            throw std::runtime_error("JSON 嵌套层级超出最大限制");
        }
        skipWhitespace(s, pos);
        if (pos >= s.size()) {
            throw std::runtime_error("JSON 意外结束");
        }
        char c = s[pos];
        if (c == '{') return parseObject(s, pos, depth + 1);
        if (c == '[') return parseArray(s, pos, depth + 1);
        if (c == '"') return JsonValue(parseString(s, pos));
        if (c == 't' || c == 'f') return parseBool(s, pos);
        if (c == 'n') return parseNull(s, pos);
        if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) return parseNumber(s, pos);
        throw std::runtime_error(std::string("非法 JSON 字符: ") + c);
    }

    static JsonValue parseObject(const std::string& s, size_t& pos, int depth) {
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
            obj[key] = parseValue(s, pos, depth);
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

    static JsonValue parseArray(const std::string& s, size_t& pos, int depth) {
        ArrayType arr;
        ++pos; // skip '['
        skipWhitespace(s, pos);
        if (pos < s.size() && s[pos] == ']') {
            ++pos;
            return JsonValue(std::move(arr));
        }
        while (pos < s.size()) {
            arr.push_back(parseValue(s, pos, depth));
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

    static void appendUtf8Codepoint(std::string& out, uint32_t cp) {
        if (cp <= 0x7F) {
            out.push_back(static_cast<char>(cp));
        } else if (cp <= 0x7FF) {
            out.push_back(static_cast<char>(0xC0 | ((cp >> 6) & 0x1F)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else if (cp <= 0xFFFF) {
            out.push_back(static_cast<char>(0xE0 | ((cp >> 12) & 0x0F)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else {
            out.push_back(static_cast<char>(0xF0 | ((cp >> 18) & 0x07)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        }
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
                    uint32_t cp = static_cast<uint32_t>(std::stoul(s.substr(pos, 4), nullptr, 16));
                    pos += 4;
                    // 处理 UTF-16 高低代理对 (Surrogate Pair, 如 Emoji \uD83D\uDE00)
                    if (cp >= 0xD800 && cp <= 0xDBFF) {
                        if (pos + 6 <= s.size() && s[pos] == '\\' && s[pos + 1] == 'u') {
                            uint32_t low = static_cast<uint32_t>(std::stoul(s.substr(pos + 2, 4), nullptr, 16));
                            if (low >= 0xDC00 && low <= 0xDFFF) {
                                pos += 6;
                                cp = 0x10000 + (((cp - 0xD800) << 10) | (low - 0xDC00));
                            }
                        }
                    }
                    appendUtf8Codepoint(out, cp);
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
