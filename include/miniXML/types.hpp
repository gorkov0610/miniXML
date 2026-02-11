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
        std::string prefix;
        std::string url;
    };
    struct attribute{
        std::string qualifiedName;
        std::string localName;
        std::string value;
        namespaceInfo* ns;
    };
    
};