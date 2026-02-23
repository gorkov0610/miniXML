#pragma once
#include "document.hpp"
#include "entities.hpp"
#include <functional>

// Internal implementation of miniXML::document parsing and serialization.
// Separated to keep document.hpp minimal and stable.
namespace miniXML{
    inline void document::tokenize(){
        size_t i = 0;
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
        size_t i{0};
        while(i < tokens.size()){
            if(i + 1 < tokens.size() && tokens[i].type == details::token_type::lt && tokens[i + 1].type == details::token_type::identifier){
                auto child = parseElement(i);

                if(child){
                    root.appendChild(std::move(child));
                }
            }else if(i + 1 < tokens.size() && tokens[i].type == details::token_type::lt && tokens[i + 1].type == details::token_type::exclamation){
                i += 4;// skip <!--
                std::string comment;
                while(i + 2 < tokens.size() && !(tokens[i].type == details::token_type::dash && tokens[i + 1].type == details::token_type::dash && tokens[i + 2].type == details::token_type::gt)){
                    comment += tokens[i++].value + " ";
                }
                i += 3;// skip -->
                auto commentChild = std::make_unique<commentNode>(comment);
                root.appendChild(std::move(commentChild));
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
                auto piNode = std::make_unique<processingInstructionNode>(pi);
                root.appendChild(std::move(piNode));
            }else{
                i++;// go on the next token
            }
        }
    }

    inline std::unique_ptr<node> document::parseElement(size_t& i){
        if(i + 1 >= tokens.size() || tokens[i].type != details::token_type::lt){
            return nullptr;
        }
        ++i;

        std::string name = tokens[i++].value;
        auto element = std::make_unique<elementNode>(name);
        while(i + 1 < tokens.size() && tokens[i].type == details::token_type::identifier && tokens[i + 1].type == details::token_type::equals){
            std::string attName = tokens[i++].value;
            ++i;// skip the =
            std::string attValue = tokens[i++].value;

            if(attName == "xmlns"){
                element->registerNamespace("", attValue);
            }else if(attName.rfind("xmlns:", 0) == 0){
                std::string prefix = attName.substr(6);
                element->registerNamespace(prefix, attValue);
            }else{
                element->appendAttribute(attName, attValue);
            }
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
            while(i + 2 < tokens.size() && !(tokens[i].type == details::token_type::dash && tokens[i + 1].type == details::token_type::dash && tokens[i + 2].type == details::token_type::gt)){ 
                comment += tokens[i++].value + " ";
            }
            i += 3;// skip -->
            auto commentChild= std::make_unique<commentNode>(comment);
            element->appendChild(std::move(commentChild));
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
                text = miniXML::details::decodeEntities(text);
                element->appendChild(std::make_unique<textNode>(text));
            }else if(i + 1 < tokens.size() && tokens[i].type == details::token_type::lt && tokens[i + 1].type == details::token_type::exclamation){
                i += 4;//skip <!--
                std::string comment;
                while(i + 2 < tokens.size() && !(tokens[i].type == details::token_type::dash && tokens[i + 1].type == details::token_type::dash && tokens[i + 2].type == details::token_type::gt)){
                    comment += tokens[i++].value + " ";
                }
                i += 3;// skip -->
                auto commentChild = std::make_unique<commentNode>(comment);
                element->appendChild(std::move(commentChild));
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
                auto piNode = std::make_unique<processingInstructionNode>(pi);
                element->appendChild(std::move(piNode));
            }else{
                i++;
            }
        }
        
        if(i + 2 < tokens.size() && tokens[i].type == details::token_type::lt && tokens[i + 1].type == details::token_type::slash && tokens[i + 2].type == details::token_type::identifier){
            std::string closing_name = tokens[i + 2].value;

            if(closing_name != name){
                throw std::runtime_error("Mismatched closing tag: " + name);
            }
            i += 3; // skip </name

            if(i < tokens.size() && tokens[i].type == details::token_type::gt){
                i++; // skip >
            }
        }
        return element;
    }

    inline void document::resolveElementNamespace(elementNode* n){
        if(!n){
            return ;
        }
        std::string prefix = n->getPrefix();
        auto current = n;
        while(current){
            auto it = current->namespaces.find(prefix);
            if(it != current->namespaces.end()){
                 if(it->second->url.empty()){
                    n->ns = nullptr;
                }else{
                    n->ns = it->second.get();
                }   
                return ;
            }
            auto parent = current->getParent();
            if(!parent || parent->getType() != details::node_type::ELEMENT_NODE){
                current = nullptr;
            }else{
                current = dynamic_cast<elementNode*>(parent);
            }
            
        }
        n->ns = nullptr;
    }
    inline void document::resolveAttributeNamespace(elementNode* n){
        if(!n){
            return ;
        }

        for(auto& a : n->getAttributes()){
            if(a.prefix.empty()){
                a.ns = nullptr;
                continue;
            }
            std::string prefix = a.prefix;

            auto current = n;
            while(current){
                auto it = current->namespaces.find(prefix);
                if(it != current->namespaces.end()){
                    a.ns = it->second.get();
                    break;
                }   
                auto parent = current->getParent();
                if(!parent || parent->getType() != details::node_type::ELEMENT_NODE){
                    current = nullptr;
                }else{
                    current = dynamic_cast<elementNode*>(parent);
                } 
                
            }

            if(!current){
                a.ns = nullptr;
            }
        }
    }
    inline void document::resolveAllNamespaces(){
        std::function<void(node *)> resolve = [&](node* n){
            if(!n || n->getType() != details::node_type::ELEMENT_NODE){
                return ;
            }

            auto e = dynamic_cast<elementNode*>(n);
            resolveElementNamespace(e);
            resolveAttributeNamespace(e);

            for(auto& c : e->getChildren()){
                resolve(c.get());
            }
        };
        resolve(&root);
    }
};