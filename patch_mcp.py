import re
import os

h_path = 'main/mcp_server.h'
cc_path = 'main/mcp_server.cc'

with open(h_path, 'r', encoding='utf-8') as f:
    h_content = f.read()

# Refactor mcp_server.h
h_content = h_content.replace('#include <cJSON.h>\n', '#include <nlohmann/json.hpp>\n')
h_content = h_content.replace('#include "cjson_utils.h"\n', '')
h_content = re.sub(r'std::string to_json\(\) const \{[\s\S]*?\}', '''nlohmann::json to_json_obj() const {
        nlohmann::json j;
        j["type"] = "image";
        j["mimeType"] = mime_type_;
        j["data"] = encoded_data_;
        return j;
    }''', h_content, count=1) # Replace ImageContent to_json

h_content = h_content.replace('using ReturnValue = std::variant<bool, int, std::string, cJSON*, ImageContent*>;',
                              'using ReturnValue = std::variant<bool, int, std::string, nlohmann::json, ImageContent*>;')

# Property::to_json
h_content = re.sub(r'std::string to_json\(\) const \{\s*CJsonUniquePtr json\(cJSON_CreateObject\(\)\);[\s\S]*?\}', '''nlohmann::json to_json_obj() const {
        nlohmann::json j;
        if (type_ == kPropertyTypeBoolean) {
            j["type"] = "boolean";
            if (has_default_value_) {
                j["default"] = value<bool>();
            }
        } else if (type_ == kPropertyTypeInteger) {
            j["type"] = "integer";
            if (has_default_value_) {
                j["default"] = value<int>();
            }
            if (min_value_.has_value()) {
                j["minimum"] = min_value_.value();
            }
            if (max_value_.has_value()) {
                j["maximum"] = max_value_.value();
            }
        } else if (type_ == kPropertyTypeString) {
            j["type"] = "string";
            if (has_default_value_) {
                j["default"] = value<std::string>();
            }
            if (max_length_.has_value()) {
                j["maxLength"] = max_length_.value();
            }
        }
        return j;
    }''', h_content, count=1)

# PropertyList::to_json
h_content = re.sub(r'std::string to_json\(\) const \{\s*CJsonUniquePtr json\(cJSON_CreateObject\(\)\);[\s\S]*?\}', '''nlohmann::json to_json_obj() const {
        nlohmann::json j = nlohmann::json::object();
        for (const auto& property : properties_) {
            j[property.name()] = property.to_json_obj();
        }
        return j;
    }''', h_content, count=1)

# McpTool::to_json
h_content = re.sub(r'std::string to_json\(\) const \{\s*std::vector<std::string> required = properties_\.GetRequired\(\);[\s\S]*?return json_str != nullptr \? std::string\(json_str\.get\(\)\) : std::string\(\);\s*\}', '''std::string to_json() const {
        std::vector<std::string> required = properties_.GetRequired();
        nlohmann::json j;
        j["name"] = name_;
        j["description"] = description_;
        
        nlohmann::json input_schema;
        input_schema["type"] = "object";
        input_schema["properties"] = properties_.to_json_obj();
        
        if (!required.empty()) {
            input_schema["required"] = required;
        }
        
        j["inputSchema"] = input_schema;

        if (user_only_) {
            j["annotations"]["audience"] = nlohmann::json::array({"user"});
        }
        
        return j.dump();
    }''', h_content, count=1)

# McpTool::Call
h_content = re.sub(r'CJsonUniquePtr owned_json;\s*if \(std::holds_alternative<ImageContent\*>\(return_value\)\) \{\s*owned_image\.reset\(std::get<ImageContent\*>\(return_value\)\);\s*\} else if \(std::holds_alternative<cJSON\*>\(return_value\)\) \{\s*owned_json\.reset\(std::get<cJSON\*>\(return_value\)\);\s*\}[\s\S]*?return std::string\(json_str\.get\(\)\);\s*\}', '''if (std::holds_alternative<ImageContent*>(return_value)) {
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
    }''', h_content)

h_content = h_content.replace('void ParseMessage(const cJSON* json, ResponseSender response_sender = nullptr);', 'void ParseMessage(const nlohmann::json& json, ResponseSender response_sender = nullptr);')
h_content = h_content.replace('void ParseCapabilities(const cJSON* capabilities);', 'void ParseCapabilities(const nlohmann::json& capabilities);')
h_content = h_content.replace('void DoToolCall(int id, const std::string& tool_name, const cJSON* tool_arguments,', 'void DoToolCall(int id, const std::string& tool_name, const nlohmann::json& tool_arguments,')

with open(h_path, 'w', encoding='utf-8') as f:
    f.write(h_content)


with open(cc_path, 'r', encoding='utf-8') as f:
    cc_content = f.read()

cc_content = re.sub(r'cJSON\*\s*json\s*=\s*cJSON_CreateObject\(\);\s*cJSON_AddNumberToObject\(json,\s*"width",\s*display->width\(\)\);\s*cJSON_AddNumberToObject\(json,\s*"height",\s*display->height\(\)\);\s*cJSON_AddBoolToObject\(json,\s*"monochrome",\s*display->IsMonochrome\(\)\);\s*return\s*json;', '''nlohmann::json json;
                            json["width"] = display->width();
                            json["height"] = display->height();
                            json["monochrome"] = display->IsMonochrome();
                            return json;''', cc_content)

cc_content = re.sub(r'void McpServer::ParseMessage\(const std::string& message, ResponseSender response_sender\) \{[\s\S]*?\}', '''void McpServer::ParseMessage(const std::string& message, ResponseSender response_sender) {
    try {
        nlohmann::json json = nlohmann::json::parse(message);
        ParseMessage(json, std::move(response_sender));
    } catch (const nlohmann::json::parse_error& e) {
        ESP_LOGE(TAG, "Failed to parse MCP message: %s", e.what());
    }
}''', cc_content, count=1)

cc_content = re.sub(r'void McpServer::ParseCapabilities\(const cJSON\* capabilities\) \{[\s\S]*?\}', '''void McpServer::ParseCapabilities(const nlohmann::json& capabilities) {
    if (capabilities.contains("vision") && capabilities["vision"].is_object()) {
        auto vision = capabilities["vision"];
        if (vision.contains("url") && vision["url"].is_string()) {
            auto camera = Board::GetInstance().GetCamera();
            if (camera) {
                std::string url_str = vision["url"].get<std::string>();
                std::string token_str;
                if (vision.contains("token") && vision["token"].is_string()) {
                    token_str = vision["token"].get<std::string>();
                }
                camera->SetExplainUrl(url_str, token_str);
            }
        }
    }
}''', cc_content, count=1)

cc_content = re.sub(r'void McpServer::ParseMessage\(const cJSON\* json, ResponseSender response_sender\) \{[\s\S]*?\}', '''void McpServer::ParseMessage(const nlohmann::json& json, ResponseSender response_sender) {
    if (!json.contains("jsonrpc") || !json["jsonrpc"].is_string() || json["jsonrpc"] != "2.0") {
        ESP_LOGE(TAG, "Invalid JSONRPC version");
        return;
    }

    if (!json.contains("method") || !json["method"].is_string()) {
        ESP_LOGE(TAG, "Missing method");
        return;
    }

    auto method_str = json["method"].get<std::string>();
    if (method_str.find("notifications") == 0) {
        return;
    }

    if (json.contains("params") && !json["params"].is_object()) {
        ESP_LOGE(TAG, "Invalid params for method: %s", method_str.c_str());
        return;
    }

    if (!json.contains("id") || !json["id"].is_number()) {
        ESP_LOGE(TAG, "Invalid id for method: %s", method_str.c_str());
        return;
    }
    int id_int = json["id"].get<int>();

    const nlohmann::json& params = json.contains("params") ? json["params"] : nlohmann::json::object();

    if (method_str == "initialize") {
        if (params.contains("capabilities") && params["capabilities"].is_object()) {
            ParseCapabilities(params["capabilities"]);
        }
        auto app_desc = esp_app_get_description();
        std::string message =
            "{\\"protocolVersion\\":\\"2024-11-05\\",\\"capabilities\\":{\\"tools\\":{}},\\"serverInfo\\":{"
            "\\"name\\":\\"" BOARD_NAME "\\",\\"version\\":\\"";
        message += app_desc->version;
        message += "\\"}}";
        ReplyResult(id_int, message, response_sender);
    } else if (method_str == "tools/list") {
        std::string cursor_str = "";
        bool list_user_only_tools = false;
        if (params.contains("cursor") && params["cursor"].is_string()) {
            cursor_str = params["cursor"].get<std::string>();
        }
        if (params.contains("withUserTools") && params["withUserTools"].is_boolean()) {
            list_user_only_tools = params["withUserTools"].get<bool>();
        }
        GetToolsList(id_int, cursor_str, list_user_only_tools, response_sender);
    } else if (method_str == "tools/call") {
        if (!params.contains("name") || !params["name"].is_string()) {
            ESP_LOGE(TAG, "tools/call: Missing tool name");
            ReplyError(id_int, -32602, "Missing tool name", response_sender);
            return;
        }
        std::string tool_name = params["name"].get<std::string>();
        
        nlohmann::json tool_arguments = nlohmann::json::object();
        if (params.contains("arguments")) {
            if (!params["arguments"].is_object()) {
                ESP_LOGE(TAG, "tools/call: Invalid arguments");
                ReplyError(id_int, -32602, "Invalid arguments: expected object", response_sender);
                return;
            }
            tool_arguments = params["arguments"];
        }
        
        DoToolCall(id_int, tool_name, tool_arguments, std::move(response_sender));
    } else {
        ESP_LOGE(TAG, "Method not implemented: %s", method_str.c_str());
        ReplyError(id_int, -32601, "Method not implemented: " + method_str, response_sender);
    }
}''', cc_content, count=1)

cc_content = re.sub(r'void McpServer::DoToolCall\(int id, const std::string& tool_name, const cJSON\* tool_arguments,\s*ResponseSender response_sender\) \{[\s\S]*?        if \(!argument\.has_default_value\(\) && !found\) \{', '''void McpServer::DoToolCall(int id, const std::string& tool_name, const nlohmann::json& tool_arguments,
                           ResponseSender response_sender) {
    auto tool_iter = std::find_if(tools_.begin(), tools_.end(), [&tool_name](const auto& tool) {
        return tool->name() == tool_name;
    });

    if (tool_iter == tools_.end()) {
        ESP_LOGE(TAG, "tools/call: Unknown tool: %s", tool_name.c_str());
        ReplyError(id, -32602, "Unknown tool: " + tool_name, response_sender);
        return;
    }

    McpTool* tool = tool_iter->get();
    PropertyList arguments = tool->properties();
    for (auto& argument : arguments) {
        bool found = false;
        std::expected<void, std::string> validation;
        if (tool_arguments.contains(argument.name())) {
            const auto& value = tool_arguments[argument.name()];
            if (argument.type() == kPropertyTypeBoolean && value.is_boolean()) {
                validation = argument.set_value<bool>(value.get<bool>());
                found = true;
            } else if (argument.type() == kPropertyTypeInteger && value.is_number_integer()) {
                validation = argument.set_value<int>(value.get<int>());
                found = true;
            } else if (argument.type() == kPropertyTypeString && value.is_string()) {
                validation = argument.set_value<std::string>(value.get<std::string>());
                found = true;
            } else {
                ESP_LOGE(TAG, "tools/call: Invalid type for argument: %s", argument.name().c_str());
                ReplyError(id, -32602, "Invalid type for argument: " + argument.name(),
                           response_sender);
                return;
            }
        }

        if (found && !validation) {
            ESP_LOGE(TAG, "tools/call: %s", validation.error().c_str());
            ReplyError(id, -32602, validation.error(), response_sender);
            return;
        }

        if (!argument.has_default_value() && !found) {''', cc_content, count=1)

with open(cc_path, 'w', encoding='utf-8') as f:
    f.write(cc_content)

print("Patch applied")
