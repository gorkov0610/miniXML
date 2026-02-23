#pragma once
#include <string>
namespace miniXML::details{
    std::string encodeEntities(std::string_view text){
        std::string result;
        result.reserve(text.size());
        for(auto c : text){
            switch (c){
            case '>':
                result += "&gt;";
                break;
            case '<':
                result += "&lt;";
                break;
            case '&':
                result += "&amp;";
                break;
            case '"':
                result += "&quot;";
                break;
            case '\'':
                result += "&apos;";
                break;
            default:
                result += c;
                break;
            }
        }
        return result;
    }

    std::string decodeEntities(std::string_view text){
        std::string result;
        result.reserve(text.size());
        for(size_t i{0}; i < text.size(); i++){
            if(text[i] == '&'){
                if(text.compare(i, 5, "&amp;") == 0){
                    result += "&";
                    i += 4;
                }else if(text.compare(i, 4, "&lt;") == 0){
                    result += "<";
                    i += 3;
                }else if(text.compare(i, 4, "&gt;") == 0){
                    result += ">";
                    i += 3;
                }else if(text.compare(i, 6, "&quot;") == 0){
                    result += "\"";
                    i += 5;
                }else if(text.compare(i, 6, "&apos;") == 0){
                    result += "'";
                    i += 5;
                }else{
                    result += "&";
                }
            }else{
                result += text[i];
            }
        }
        return result;
    }
};