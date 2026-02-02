#pragma once
#include "document.hpp"
// Internal implementation of miniXML::document parsing and serialization.
// Separated to keep document.hpp minimal and stable.
namespace miniXML{
    inline void document::tokenize(){
        int i = 0;
        while(i < content.size()){
            std::string s;
            char quote;
            switch (content[i]){
                case '<':
                    tokens.push_back({details::token_type::lt, "<"});
                    i++;
                    break;
                case '>':
                    tokens.push_back({details::token_type::gt, ">"});
                    i++;
                    break;
                case '=':
                    tokens.push_back({details::token_type::equals, "="});
                    i++;
                    break;
                case '"': case '\'':
                    quote = content[i++];
                    while(i < content.size() && content[i] != quote){
                        s += content[i++];
                    }
                    ++i;
                    tokens.push_back({details::token_type::string, s}); 
                    break;
                case '/':
                    tokens.push_back({details::token_type::slash, "/"});
                    i++;
                    break;
                case '?':
                    tokens.push_back({details::token_type::question, "?"});
                    i++;
                    break;
                case '-':
                    tokens.push_back({details::token_type::dash, "-"});
                    i++;
                    break;
                case '!':
                    tokens.push_back({details::token_type::exclamation, "!"});
                    i++;
                    break;
                default:
                    if(std::isspace(static_cast<unsigned char>(content[i]))){
                        i++;
                    }else{
                        while(i < content.size() && !std::isspace(content[i]) && content[i] != '<' && content[i] != '>' && content[i] != '=' && content[i] != '/' && content[i] != '?' && content[i] != '-'){
                            s += content[i++];
                        }
                        tokens.push_back({details::token_type::identifier, s});
                    }
                    break;

            }
        }
    }

    inline void document::buildTree(){
        auto i{0};
        while(i < tokens.size()){
            if(i + 1 < tokens.size() && tokens[i].type == details::token_type::lt && tokens[i + 1].type == details::token_type::identifier){
                auto child = parseElement(i);
                if(child){
                    root.appendChild(std::move(child));
                }
            }else if(i + 1 < tokens.size() && tokens[i].type == details::token_type::lt && tokens[i + 1].type == details::token_type::exclamation){
                i += 4;// skip <!--
                std::string comment;
                while(i + 2 < tokens.size() && tokens[i].type != details::token_type::dash && tokens[i + 1].type != details::token_type::dash && tokens[i + 2].type != details::token_type::gt){
                    comment += tokens[i++].value + " ";
                }
                i += 3;// skip -->
                auto commentNode = std::make_unique<node>(details::node_type::COMMENT_NODE, comment);
                root.appendChild(std::move(commentNode));
            }else if(i + 1 < tokens.size() && tokens[i].type == details::token_type::lt && tokens[i + 1].type == details::token_type::question){
                i += 2;// skip <?
                std::string pi;
                while(i + 1 < tokens.size() && tokens[i].type != details::token_type::question){
                    if(tokens[i].type == details::token_type::identifier && tokens[i + 1].type == details::token_type::equals){
                        pi += " " + tokens[i].value;
                    }else if(tokens[i].type == details::token_type::equals){
                        pi += tokens[i].value + "\"";
                    }else if(tokens[i].type == details::token_type::string){
                        pi += tokens[i].value + "\"";
                    }else{
                        pi += tokens[i].value;
                    }
                    i++;
                }
                auto piNode = std::make_unique<node>(details::node_type::PROCESSING_INSTRUCTION_NODE, pi);
                root.appendChild(std::move(piNode));
            }else{
                i++;// go on the next token
            }
        }
    }

    inline std::unique_ptr<node> document::parseElement(int& i){
        if(i + 1 >= tokens.size() || tokens[i].type != details::token_type::lt){
            return nullptr;
        }
        ++i;

        std::string name = tokens[i++].value;
        auto element = std::make_unique<node>(details::node_type::ELEMENT_NODE, name);
        while(i + 1 < tokens.size() && tokens[i].type == details::token_type::identifier && tokens[i + 1].type == details::token_type::equals){
            std::string attName = tokens[i++].value;
            ++i;// skip the =
            std::string attValue = tokens[i++].value;
            element->appendAttribute(attName, attValue);
        }
        if(i + 1 < tokens.size() && tokens[i].type == details::token_type::slash && tokens[i + 1].type == details::token_type::gt){
            i += 2;// skip />
            return element;
        }
        if(i < tokens.size() && tokens[i].type == details::token_type::gt){
            i++;
        }
        if(i + 1 < tokens.size() && tokens[i].type == details::token_type::lt && tokens[i + 1].type == details::token_type::exclamation){
            i += 4;// skip <!--
            std::string comment;
            while(i + 2 < tokens.size() && tokens[i].type != details::token_type::dash && tokens[i + 1].type != details::token_type::dash && tokens[i + 2].type != details::token_type::gt){
                    comment += tokens[i++].value + " ";
            }
            i += 3;// skip -->
            auto commentNode = std::make_unique<node>(details::node_type::COMMENT_NODE, comment);
            element->appendChild(std::move(commentNode));
        }

        while(i + 1 < tokens.size() && !(tokens[i].type == details::token_type::lt && tokens[i + 1].type == details::token_type::slash)){
            if(i + 1 < tokens.size() && tokens[i].type == details::token_type::lt && (tokens[i + 1].type == details::token_type::identifier || tokens[i + 1].type == details::token_type::string)){
                element->appendChild(parseElement(i));
            }else if(i + 1 < tokens.size() && tokens[i].type == details::token_type::identifier && tokens[i + 1].type != details::token_type::dash){
                std::string text;
                while(i < tokens.size() && (tokens[i].type == details::token_type::string || tokens[i].type == details::token_type::identifier)){
                    text += tokens[i++].value + ' ';
                }
                if(!text.empty()){
                    text.pop_back();
                }
                element->appendChild(std::make_unique<node>(details::node_type::TEXT_NODE, text));
            }else if(i + 1 < tokens.size() && tokens[i].type == details::token_type::lt && tokens[i + 1].type == details::token_type::exclamation){
                i += 4;//skip <!--
                std::string comment;
                while(i + 2 < tokens.size() && tokens[i].type != details::token_type::dash && tokens[i + 1].type != details::token_type::dash && tokens[i + 2].type != details::token_type::gt){
                    comment += tokens[i++].value + " ";
                }
                i += 3;// skip -->
                auto commentNode = std::make_unique<node>(details::node_type::COMMENT_NODE, comment);
                element->appendChild(std::move(commentNode));
            }else if(i + 1 < tokens.size() && tokens[i].type == details::token_type::lt && tokens[i + 1].type == details::token_type::question){
                i += 2;// skip <?
                std::string pi;
                while(i + 1 < tokens.size() && tokens[i].type != details::token_type::question){
                    if(tokens[i].type == details::token_type::identifier && tokens[i + 1].type == details::token_type::equals){
                        pi += " " + tokens[i].value;
                    }else if(tokens[i].type == details::token_type::equals){
                        pi += tokens[i].value + "\"";
                    }else if(tokens[i].type == details::token_type::string){
                        pi += tokens[i].value + "\"";
                    }else{
                        pi += tokens[i].value;
                    }
                    i++;
                }
                auto piNode = std::make_unique<node>(details::node_type::PROCESSING_INSTRUCTION_NODE, pi);
                element->appendChild(std::move(piNode));
            }else{
                i++;
            }
        }
        
        if(i + 2 < tokens.size() && tokens[i].type == details::token_type::lt && tokens[i + 1].type == details::token_type::slash && tokens[i + 2].type == details::token_type::identifier){
            std::string closing_name = tokens[i + 2].value;

            if(closing_name != name){
                throw std::runtime_error("Closing tag of " + name + " doesn't match!");
            }
            i += 3; // skip </name

            if(i < tokens.size() && tokens[i].type == details::token_type::gt){
                i++; // skip >
            }
        }
        return element;
    }

    inline void document::writeNode(const node& n, std::ostream& file, int depth = 0) const {
        const std::string ind(depth * 2, ' ');
        switch (n.getType()){
        case details::node_type::ELEMENT_NODE:{
            file << ind << "<" << n.getValue();
            
            for(const auto& a : n.getAttributes()){
                file << ' ' << a.first << "=\"";
                if(!a.second.empty()){
                    file << a.second << "\"";
                }else{
                    file << "\"";
                }
            }

            if(n.getChildren().empty()){
                file << "/>\n";
                return;    
            }
            
            file << ">\n";

            for(const auto& c : n.getChildren()){
                writeNode(*c, file, depth + 1);
            }

            file << ind << "</" + n.getValue() + ">\n";
            break;
        }
        case details::node_type::COMMENT_NODE:{
            std::string comment = n.getValue();
            if(!comment.empty()){
                comment.pop_back();// it allways has a whitespace at the end, due to the parsing
            }
            file << ind << "<!--" << comment << "-->" << "\n";
            break;
        }
        case details::node_type::TEXT_NODE:{
            file << ind << n.getValue() << "\n";
            break;
        }
        case details::node_type::PROCESSING_INSTRUCTION_NODE:{
            file << ind << "<?" << n.getValue() << "?>\n";
            break;
        }
        default:
            break;
        }
    }
};