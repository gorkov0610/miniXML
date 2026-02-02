#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include <optional>
#include <algorithm>
#include "types.hpp"

namespace miniXML{  
    class node{
        public:
            //constructor
            node(details::node_type t, std::string v) : type(t), value(std::move(v)){}

            details::node_type getType() const noexcept {
                return type;
            }
            const std::string& getValue() const noexcept {
                return value;
            }
            const std::vector<std::unique_ptr<node>>& getChildren() const {
                return children;
            }
            node* getParent() noexcept {
                return parent;
            }
            const node* getParent() const noexcept {
                return parent;
            }
            const std::unordered_map<std::string, std::string>& getAttributes() const{
                return attributes;
            }
            void setType(const details::node_type n) noexcept {
                type = n;
            }
            void setValue(const std::string_view n){
                value.assign(n);
            }
            [[nodiscard]] std::string toString(int depth = 0) const {
                const std::string ind(depth * 2, ' ');
                std::string xml;
                xml += ind;
                switch (type){
                    case details::node_type::TEXT_NODE:{
                        xml += value + '\n';
                        break;
                    }
                    case details::node_type::PROCESSING_INSTRUCTION_NODE:{
                        xml += "<?" + value + "?>\n";
                        break;
                    }
                    case details::node_type::COMMENT_NODE:{
                        std::string comment = value;
                        if(!comment.empty()){
                            comment.pop_back();
                        }
                        xml += "<!--" + comment + "-->\n";
                        break;
                    }
                    case details::node_type::ELEMENT_NODE:{
                        std::string element = "<" + value;

                        for(const auto& a : attributes){
                            element += " " + a.first + "=\"";
                            element += a.second;
                            element += "\"";
                        }
            
                        if(children.empty()){
                            element += "/>\n";
                            xml += element;
                            return xml;
                        }

                        element += ">\n";

                        for(const auto& c : children){
                            element += c->toString(depth + 1);
                        }

                        element += ind +"</" + value + ">\n";
                        xml += element;

                        break;
                    }
                    default:{
                        for(const auto& c : children){
                            xml += c->toString(depth); 
                        }
                        break;
                    }    
                }
                return xml;
            }
            void appendAttribute(const std::string_view key, const std::string_view value){
                attributes.insert_or_assign(std::string(key), std::string(value));
            }
            node* appendChild(std::unique_ptr<node> n){
                n->parent = this;
                children.push_back(std::move(n));
                return children.back().get();
            }
            [[nodiscard]] bool deleteAttribute(const std::string& key){
                auto it = attributes.find(key);
                if(it == attributes.end()){
                    return false;
                }
                attributes.erase(it);
                return true;
            }
            [[nodiscard]] bool deleteChild(const node* n){
                if(!n){
                    return false;
                }

                auto it = std::find_if(children.begin(), children.end(), [n](const std::unique_ptr<node>& c){
                    return n == c.get();
                });
                
                if(it == children.end()){
                    return false;
                }

                (*it)->parent = nullptr;
                children.erase(it);
                return true;
            }
            [[nodiscard]] bool deleteFromParent(){
                return parent ? parent->deleteChild(this) : false;
            }
            void clearChildren(){
                for(auto& c : children){
                    c->parent = nullptr;
                }
                children.clear();
            }
            void clearAttributes(){
                attributes.clear();
            }
            [[nodiscard]] std::optional<std::string> getAttribute(const std::string& key) const{
                auto it = attributes.find(key);
                if(it == attributes.end()){
                    return std::nullopt;
                }
                return it->second;
            }
            [[nodiscard]] node* findChild(const details::node_type t){
                const auto it = std::find_if(children.begin(), children.end(), [t](const std::unique_ptr<node>&n){
                    return n->type == t;
                });

                return it != children.end() ? it->get() : nullptr;
            }
            [[nodiscard]] const node* findChild(const details::node_type t) const {
                const auto it = std::find_if(children.begin(), children.end(), [t](const std::unique_ptr<node>&n){
                    return n->type == t;
                });

                return it != children.end() ? it->get() : nullptr;
            }
            [[nodiscard]] node* findChild(const std::string& t){
                const auto it = std::find_if(children.begin(), children.end(), [t](const std::unique_ptr<node>&n){
                    return n->value == t;
                });

                return it != children.end() ? it->get() : nullptr;
            }
            [[nodiscard]] const node* findChild(const std::string& t) const {
                const auto it = std::find_if(children.begin(), children.end(), [t](const std::unique_ptr<node>&n){
                    return n->value == t;
                });

                return it != children.end() ? it->get() : nullptr;
            }
            [[nodiscard]] std::vector<node*> findChildren(const std::string& v){
                std::vector<node*> results;
                
                for(const auto& c : children){
                    if(c->value == v){
                        results.push_back(c.get());
                    }
                }
                return results;
            }
            [[nodiscard]] std::vector<const node*> findChildren(const std::string& v) const {
                std::vector<const node*> results;
                
                for(const auto& c : children){
                    if(c->value == v){
                        results.push_back(c.get());
                    }
                }
                return results;
            }
            [[nodiscard]] std::vector<node*> findChildren(details::node_type t){
                std::vector<node*> results;
                
                for(const auto& c : children){
                    if(c->type == t){
                        results.push_back(c.get());
                    }
                }
                return results;
            }
            [[nodiscard]] std::vector<const node*> findChildren(details::node_type t) const {
                std::vector<const node*> results;
                
                for(const auto& c : children){
                    if(c->type == t){
                        results.push_back(c.get());
                    }
                }
                return results;
            }
        private:
            details::node_type type;
            std::string value;
            std::vector<std::unique_ptr<node>> children;
            std::unordered_map<std::string, std::string> attributes;
            node* parent = nullptr;

            friend class document;
    };
};