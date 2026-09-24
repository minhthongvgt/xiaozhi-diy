#ifndef MCP_SERVER_H
#define MCP_SERVER_H

#include <expected>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>
#include <initializer_list>

#include <esp_system.h>
#include <mbedtls/base64.h>
#include <nlohmann/json.hpp>

class ImageContent {
private:
    std::string encoded_data_;
    std::string mime_type_;

    static std::string Base64Encode(const std::string& data) {
        size_t dlen = 0, olen = 0;
        mbedtls_base64_encode((unsigned char*)nullptr, 0, &dlen, (const unsigned char*)data.data(),
                              data.size());
        std::string result(dlen, 0);
        mbedtls_base64_encode((unsigned char*)result.data(), result.size(), &olen,
                              (const unsigned char*)data.data(), data.size());
        return result;
    }

public:
    ImageContent(const std::string& mime_type, const std::string& data) {
        mime_type_ = mime_type;
        // base64 encode data
        encoded_data_ = Base64Encode(data);
    }

    nlohmann::json to_json_obj() const {
        nlohmann::json j;
        j["type"] = "image";
        j["mimeType"] = mime_type_;
        j["data"] = encoded_data_;
        return j;
    }
};

class PropertyList;

using ReturnValue = std::variant<bool, int, std::string, nlohmann::json, ImageContent*>;
using ToolResult = std::expected<ReturnValue, std::string>;
using ToolCallback = std::function<ToolResult(const PropertyList&)>;

enum PropertyType { kPropertyTypeBoolean, kPropertyTypeInteger, kPropertyTypeString };

class Property {
private:
    std::string name_;
    PropertyType type_;
    std::variant<bool, int, std::string> value_;
    bool has_default_value_;
    std::optional<int> min_value_;  // 新增：整数最小值
    std::optional<int> max_value_;  // 新增：整数最大值
    std::optional<size_t> max_length_;

public:
    // Required field constructor
    Property(const std::string& name, PropertyType type)
        : name_(name), type_(type), has_default_value_(false) {}

    // Optional field constructor with default value
    template <typename T>
    Property(const std::string& name, PropertyType type, const T& default_value)
        : name_(name), type_(type), has_default_value_(true) {
        value_ = default_value;
    }

    Property(const std::string& name, PropertyType type, int min_value, int max_value)
        : name_(name),
          type_(type),
          has_default_value_(false),
          min_value_(min_value),
          max_value_(max_value) {
        if (type != kPropertyTypeInteger) {
            esp_system_abort("MCP property range is only valid for integer properties");
        }
    }

    Property(const std::string& name, PropertyType type, int default_value, int min_value,
             int max_value)
        : name_(name),
          type_(type),
          has_default_value_(true),
          min_value_(min_value),
          max_value_(max_value) {
        if (type != kPropertyTypeInteger) {
            esp_system_abort("MCP property range is only valid for integer properties");
        }
        if (default_value < min_value || default_value > max_value) {
            esp_system_abort("MCP property default value is outside its declared range");
        }
        value_ = default_value;
    }

    // Set max_length for string properties (builder pattern)
    Property& SetMaxLength(size_t max_length) {
        if (type_ != kPropertyTypeString) {
            esp_system_abort("Max length only applies to string properties");
        }
        max_length_ = max_length;
        return *this;
    }

    inline const std::string& name() const { return name_; }
    inline PropertyType type() const { return type_; }
    inline bool has_default_value() const { return has_default_value_; }
    inline bool has_range() const { return min_value_.has_value() && max_value_.has_value(); }
    inline int min_value() const { return min_value_.value_or(0); }
    inline int max_value() const { return max_value_.value_or(0); }

    inline bool has_max_length() const { return max_length_.has_value(); }
    inline size_t max_length() const { return max_length_.value_or(0); }

    template <typename T>
    inline const T& value() const {
        auto value = std::get_if<T>(&value_);
        if (value == nullptr) {
            esp_system_abort("MCP property value type does not match its declaration");
        }
        return *value;
    }

    // Validate a value against this property's constraints without setting it.
    // Returns empty string on success, or an error message on failure.
    std::string Validate(const std::variant<bool, int, std::string>& val) const {
        if (type_ == kPropertyTypeInteger && std::holds_alternative<int>(val)) {
            int v = std::get<int>(val);
            if (min_value_.has_value() && v < min_value_.value()) {
                return "Property '" + name_ + "': value " + std::to_string(v) +
                       " is below minimum " + std::to_string(min_value_.value());
            }
            if (max_value_.has_value() && v > max_value_.value()) {
                return "Property '" + name_ + "': value " + std::to_string(v) +
                       " exceeds maximum " + std::to_string(max_value_.value());
            }
        } else if (type_ == kPropertyTypeString && std::holds_alternative<std::string>(val)) {
            const auto& s = std::get<std::string>(val);
            if (max_length_.has_value() && s.size() > max_length_.value()) {
                return "Property '" + name_ + "': string length " + std::to_string(s.size()) +
                       " exceeds maximum " + std::to_string(max_length_.value());
            }
        }
        return "";
    }

    template <typename T>
    inline std::expected<void, std::string> set_value(const T& value) {
        auto error = Validate(value);
        if (!error.empty()) {
            return std::unexpected(std::move(error));
        }
        value_ = value;
        return {};
    }

    nlohmann::json to_json_obj() const {
        nlohmann::json j;
        if (type_ == kPropertyTypeBoolean) {
            j["type"] = "boolean";
            if (has_default_value_) j["default"] = value<bool>();
        } else if (type_ == kPropertyTypeInteger) {
            j["type"] = "integer";
            if (has_default_value_) j["default"] = value<int>();
            if (min_value_.has_value()) j["minimum"] = min_value_.value();
            if (max_value_.has_value()) j["maximum"] = max_value_.value();
        } else if (type_ == kPropertyTypeString) {
            j["type"] = "string";
            if (has_default_value_) j["default"] = value<std::string>();
            if (max_length_.has_value()) j["maxLength"] = max_length_.value();
        }
        return j;
    }
};

class PropertyList {
private:
    std::vector<Property> properties_;

public:
    PropertyList() = default;
    PropertyList(std::initializer_list<Property> properties) : properties_(properties) {}
    PropertyList(const std::vector<Property>& properties) : properties_(properties) {}
    void AddProperty(const Property& property) { properties_.push_back(property); }

    const Property& operator[](const std::string& name) const {
        for (const auto& property : properties_) {
            if (property.name() == name) {
                return property;
            }
        }
        esp_system_abort("MCP property lookup failed because the property was not declared");
    }

    auto begin() { return properties_.begin(); }
    auto end() { return properties_.end(); }

    std::vector<std::string> GetRequired() const {
        std::vector<std::string> required;
        for (auto& property : properties_) {
            if (!property.has_default_value()) {
                required.push_back(property.name());
            }
        }
        return required;
    }

    nlohmann::json to_json_obj() const {
        nlohmann::json j = nlohmann::json::object();
        for (const auto& property : properties_) {
            j[property.name()] = property.to_json_obj();
        }
        return j;
    }
};

class McpTool {
private:
    std::string name_;
    std::string description_;
    PropertyList properties_;
    ToolCallback callback_;
    bool user_only_ = false;

public:
    McpTool(const std::string& name, const std::string& description, const PropertyList& properties,
            ToolCallback callback)
        : name_(name), description_(description), properties_(properties), callback_(callback) {}

    void set_user_only(bool user_only) { user_only_ = user_only; }
    inline const std::string& name() const { return name_; }
    inline const std::string& description() const { return description_; }
    inline const PropertyList& properties() const { return properties_; }
    inline bool user_only() const { return user_only_; }

    std::string to_json() const {
        nlohmann::json j;
        j["name"] = name_;
        j["description"] = description_;
        
        nlohmann::json input_schema;
        input_schema["type"] = "object";
        input_schema["properties"] = properties_.to_json_obj();
        
        std::vector<std::string> required = properties_.GetRequired();
        if (!required.empty()) {
            input_schema["required"] = required;
        }
        
        j["inputSchema"] = input_schema;

        if (user_only_) {
            j["annotations"]["audience"] = nlohmann::json::array({"user"});
        }
        
        return j.dump();
    }

    std::expected<std::string, std::string> Call(const PropertyList& properties) {
        auto callback_result = callback_(properties);
        if (!callback_result) {
            return std::unexpected(std::move(callback_result.error()));
        }
        ReturnValue return_value = std::move(*callback_result);
        std::unique_ptr<ImageContent> owned_image;
        if (std::holds_alternative<ImageContent*>(return_value)) {
            owned_image.reset(std::get<ImageContent*>(return_value));
        }

        nlohmann::json result;
        nlohmann::json content = nlohmann::json::array();
        
        if (std::holds_alternative<ImageContent*>(return_value)) {
            if (owned_image == nullptr) {
                return std::unexpected("MCP tool returned an empty image");
            }
            nlohmann::json image;
            image["type"] = "image";
            image["image"] = owned_image->to_json_obj();
            content.push_back(image);
        } else {
            nlohmann::json text;
            text["type"] = "text";
            if (std::holds_alternative<std::string>(return_value)) {
                text["text"] = std::get<std::string>(return_value);
            } else if (std::holds_alternative<bool>(return_value)) {
                text["text"] = std::get<bool>(return_value) ? "true" : "false";
            } else if (std::holds_alternative<int>(return_value)) {
                text["text"] = std::to_string(std::get<int>(return_value));
            } else if (std::holds_alternative<nlohmann::json>(return_value)) {
                text["text"] = std::get<nlohmann::json>(return_value).dump();
            }
            content.push_back(text);
        }
        
        result["content"] = content;
        result["isError"] = false;
        return result.dump();
    }
};

class McpServer {
public:
    using ResponseSender = std::function<void(const std::string&)>;
    static McpServer& GetInstance() {
        static McpServer instance;
        return instance;
    }

    void AddCommonTools();
    void AddUserOnlyTools();
    void AddTool(std::unique_ptr<McpTool> tool);
    void AddTool(const std::string& name, const std::string& description,
                 const PropertyList& properties, ToolCallback callback);
    void AddUserOnlyTool(const std::string& name, const std::string& description,
                         const PropertyList& properties, ToolCallback callback);
    void ParseMessage(const nlohmann::json& json, ResponseSender response_sender = nullptr);
    void ParseMessage(const std::string& message, ResponseSender response_sender = nullptr);

private:
    McpServer();
    ~McpServer();

    void ParseCapabilities(const nlohmann::json& capabilities);

    void SendResponse(const std::string& payload, const ResponseSender& response_sender);
    void ReplyResult(int id, const std::string& result, const ResponseSender& response_sender);
    void ReplyError(int id, const std::string& message, const ResponseSender& response_sender);
    void ReplyError(int id, int code, const std::string& message,
                    const ResponseSender& response_sender);

    void GetToolsList(int id, const std::string& cursor, bool list_user_only_tools,
                      const ResponseSender& response_sender);
    void DoToolCall(int id, const std::string& tool_name, const nlohmann::json& tool_arguments,
                    ResponseSender response_sender);

    std::vector<std::unique_ptr<McpTool>> tools_;
};

#endif  // MCP_SERVER_H
