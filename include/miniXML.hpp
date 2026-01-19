#pragma once
#include <vector>
#include <regex>
#include <string>   
#include <iostream>

namespace miniXML{
    enum node_type{
        ELEMENT_NODE = 1,
        ATTRIBUTE_NODE,
        TEXT_NODE,
        CDATA_SECTION_NODE,
        ENTITY_REFERENCE_NODE,
        ENTITY_NODE,
        PROCESSING_INSTRUCTION_NODE,
        COMMENT_NODE,
        DOCUMENT_NODE,
        DOCUMENT_TYPE_NODE, 
        DOCUMENT_FRAGMENT_NODE,
        NOTATION_NODE
    };

    class node{
        public:
            //constructor
            node(node_type t, std::string v) : type(t), value(std::move(v)){}

            node_type getType() const {
                return type;
            }
            const std::string& getValue() const{
                return value;
            }
            const std::vector<node>& getChildren() const {
                return children;
            }

            void appendChild(node n){
                children.push_back(std::move(n));
            }
            const node* findChild(const std::string& v) const {
                const auto it = std::find_if(children.begin(), children.end(), v);

                if(it != children.end()){
                    return &(*it);
                }
                return nullptr;
            }
            const node* findChild(const node_type t) const {
                const auto it = std::find_if(children.begin(), children.end(), t);

                if(it != children.end()){
                    return &(*it);
                }
                return nullptr;
            }
            std::vector<const node*> findChildren(const std::string& v) const {
                std::vector<const node*> results;
                
                for(const auto& c : children){
                    if(c == v){
                        results.push_back(&c);
                    }
                }
                return results;
            }
            void printTree(int indent = 0) const {
                std::cout << std::string(indent, ' ') << value << '\n';
                for(const auto& c : children){
                    c.printTree(indent + 2);
                }
            }

            bool operator==(const std::string& v) const {
                return value == v;
            }
            bool operator==(const node_type t) const {
                return t == type;
            }
        private:
            node_type type;
            std::string value;
            std::vector<node> children;
    };
}