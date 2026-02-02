#pragma once
#include <string>

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
};