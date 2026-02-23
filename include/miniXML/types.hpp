#pragma once
#include <string>
// Primitive internal types used across the library
// Uses a special namespace to avoid namespace pollution
namespace miniXML::details{
    enum class token_type{
        lt, gt, slash, equals,
        identifier, 
        string,
        question,
        dash,
        exclamation
    };
    enum class node_type{
        ELEMENT_NODE = 1,
        TEXT_NODE,
        DOCUMENT_NODE,
        COMMENT_NODE,
        PROCESSING_INSTRUCTION_NODE
    };

    struct token{
        token_type type;
        std::string value;
    };
    struct namespaceInfo{
        namespaceInfo(const std::string& prefix, const std::string& url) : prefix(prefix), url(url) {}
        std::string prefix;
        std::string url;
    };
    struct attribute{
        attribute(const std::string& name, const std::string& value) : qualifiedName(name), value(value){
            auto colon = name.find(':');

            if (colon == std::string::npos) {
                prefix = "";
                localName = name;
            } else {
                prefix = name.substr(0, colon);
                localName = name.substr(colon + 1);
            }
        }
        std::string qualifiedName;
        std::string localName;
        std::string prefix;
        std::string value;
        const namespaceInfo* ns = nullptr;
    };
    
};