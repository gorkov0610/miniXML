#pragma once
#include <vector>
#include <memory>
#include <string>   
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stack>

namespace miniXML{
    enum token_type{
        lt, gt, slash, equals,
        identifier, 
        string,
        text
    };
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

    struct token
    {
        token_type type;
        std::string value;
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
            const std::vector<std::unique_ptr<node>>& getChildren() const {
                return children;
            }
            void setType(const node_type n){
                type = n;
            }
            void setValue(const std::string& n){
                value = n;
            }

            node* appendChild(std::unique_ptr<node> n){
                children.push_back(std::move(n));

                return children.back().get();
            }
            bool deleteChild(const std::string& name){
                auto it = std::find_if(children.begin(), children.end(), [name](const std::unique_ptr<node>& n){
                    return name == n->value;
                });
                if(it != children.end()){
                    children.erase(it);
                    return true;
                }
                return false;
            }
            bool deleteChild(const node_type type){
                auto it = std::find_if(children.begin(), children.end(), [type](const std::unique_ptr<node>& n){
                    return type == n->type;
                });
                if(it != children.end()){
                    children.erase(it);
                    return true;
                }
                return false;
            }
            bool deleteChild(const node& n){
                auto it = std::find_if(children.begin(), children.end(), [&n](const std::unique_ptr<node>& c){
                    return n.value == c->value && n.type == c->type;
                });
                if(it != children.end()){
                    children.erase(it);
                    return true;
                }
                return false;
            }
            node* findChild(const node_type t){
                const auto it = std::find_if(children.begin(), children.end(), [t](const std::unique_ptr<node>&n){
                    return n->type == t;
                });

                return it != children.end() ? it->get() : nullptr;
            }
            const node* findChild(const node_type t) const {
                const auto it = std::find_if(children.begin(), children.end(), [t](const std::unique_ptr<node>&n){
                    return n->type == t;
                });

                return it != children.end() ? it->get() : nullptr;
            }
            node* findChild(const std::string& t){
                const auto it = std::find_if(children.begin(), children.end(), [t](const std::unique_ptr<node>&n){
                    return n->value == t;
                });

                return it != children.end() ? it->get() : nullptr;
            }
            const node* findChild(const std::string& t) const {
                const auto it = std::find_if(children.begin(), children.end(), [t](const std::unique_ptr<node>&n){
                    return n->value == t;
                });

                return it != children.end() ? it->get() : nullptr;
            }
            std::vector<node*> findChildren(const std::string& v){
                std::vector<node*> results;
                
                for(const auto& c : children){
                    if(*c == v){
                        results.push_back(c.get());
                    }
                }
                return results;
            }
            std::vector<const node*> findChildren(const std::string& v) const {
                std::vector<const node*> results;
                
                for(const auto& c : children){
                    if(*c == v){
                        results.push_back(c.get());
                    }
                }
                return results;
            }
            std::vector<node*> findChildren(node_type t){
                std::vector<node*> results;
                
                for(const auto& c : children){
                    if(*c == t){
                        results.push_back(c.get());
                    }
                }
                return results;
            }
            std::vector<const node*> findChildren(node_type t) const {
                std::vector<const node*> results;
                
                for(const auto& c : children){
                    if(*c == t){
                        results.push_back(c.get());
                    }
                }
                return results;
            }
            void printTree(int indent = 0) const {
                std::cout << std::string(indent, ' ') << value << '\n';
                for(const auto& c : children){
                    c->printTree(indent + 2);
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
            std::vector<std::unique_ptr<node>> children;
    };
    class document{
        public:
            document(const std::string& filepath) : root(DOCUMENT_NODE, "") {
                const std::ifstream f(filepath);
                std::stringstream buffer;
                
                buffer << f.rdbuf();

                file = buffer.str();
                tokenize();
                buildTree();
            }
            node& rootNode(){
                return root;
            }
            const node& rootNode() const{
                return root;
            }
            void writeToFile(const std::string& filepath){
                std::ofstream file(filepath);
                for(const auto& c : root.getChildren()){
                    writeNode(*c, file);    
                }
            }
            void writeNode(const node& n, std::ofstream& file){
                switch (n.getType())
                {
                case PROCESSING_INSTRUCTION_NODE:{
                    file << "<?" << n.getValue();

                    for(const auto& a : n.findChildren(ATTRIBUTE_NODE)){
                        file << ' ' << a->getValue() << "=\"";

                        for(const auto& t : a->getChildren()){
                            file << t->getValue();
                        }
                        file << "\"";
                    }
                    file << "?>\n";
                    break;
                }
                case ELEMENT_NODE:{
                    file << "<" << n.getValue();
                    const auto childrenElements = n.findChildren(ELEMENT_NODE);
                    const auto childrenText = n.findChildren(TEXT_NODE);

                    for(const auto& a : n.findChildren(ATTRIBUTE_NODE)){
                        file << ' ' << a->getValue() << "=\"";
                        std::string helper;
                        for(const auto& v : a->getChildren()){
                            helper += v->getValue() + ',';
                        }
                        if(!helper.empty()){
                            helper.pop_back();
                        }
                        file << helper + '\"';
                    }

                    if(childrenElements.empty() && childrenText.empty()){
                        file << "/>\n";
                        return;
                    }

                    file << ">\n";

                    for(const auto& t : childrenText){
                        file << t->getValue();
                    }

                    for(const auto& e : childrenElements){
                        writeNode(*e, file);
                    }
                    file << "</" + n.getValue() + ">\n";
                    break;
                }
                case TEXT_NODE:{
                    file << n.getValue();
                    break;
                }
                default:
                    break;
                }
            }
        private:
            node root;
            std::string file;
            
            std::vector<token> tokens;

            void tokenize(){
                int i = 0;
                while(i < file.size()){
                    std::string s;
                    char quote;
                    switch (file[i]){
                        case '<':
                            tokens.push_back({lt, "<"});
                            i++;
                            break;
                        case '>':
                            tokens.push_back({gt, ">"});
                            i++;
                            break;
                        case '=':
                            tokens.push_back({equals, "="});
                            i++;
                            break;
                        case '"': case '\'':
                            quote = file[i++];
                            while(i < file.size() && file[i] != quote){
                                s += file[i++];
                            }
                            ++i;
                            tokens.push_back({string, s});
                            break;
                        case '/':
                            tokens.push_back({slash, "/"});
                            i++;
                            break;
                        default:
                            if(std::isspace(file[i])){
                                i++;
                            }else{
                                while(i < file.size() && !std::isspace(file[i]) && file[i] != '<' && file[i] != '>' && file[i] != '=' && file[i] != '/'){
                                    s += file[i++];
                                }
                                tokens.push_back({identifier, s});
                            }
                            break;

                    }
                }
            }

            void buildTree(){
                std::stack<node*> stack;
                stack.push(&root);
                for(auto i{0}; i < tokens.size(); i++){
                    const auto& t = tokens[i];
                    if(t.type == lt){
                        if(tokens[i + 1].type == identifier && tokens[i + 1].value[0] == '?' && !tokens[i + 1].value.empty()){
                        ++i;
                        std::string pi = tokens[i].value.substr(1);

                        auto piNode = std::make_unique<node>(PROCESSING_INSTRUCTION_NODE, pi);

                        while(i + 2 < tokens.size() && tokens[i].type == identifier && tokens[i + 1].type == equals && tokens[i + 2].type == string){
                            ++i;
                            std::string attName = tokens[i].value;
                            i += 2;
                            std::string attValue = tokens[i].value;
                            auto attribute = std::make_unique<node>(ATTRIBUTE_NODE, attName);
                            attribute->appendChild(std::make_unique<node>(TEXT_NODE, attValue));
                            ++i;
                            piNode->appendChild(std::move(attribute));
                        }
                        ++i;
                        root.appendChild(std::move(piNode));
                        continue;
                    }
                        if(tokens[i + 1].type == slash) {
                            i += 2;
                            if(i + 1 < tokens.size() && tokens[i + 1].type == gt) i++;
                            if(stack.size() > 1) stack.pop();
                            continue;
                        }
                        std::string name = tokens[++i].value;

                        auto element = std::make_unique<node>(ELEMENT_NODE, name);

                        while(i + 1 < tokens.size() && tokens[i + 1].type != gt && tokens[i + 1].type != slash){
                            ++i;
                            std::string attName = tokens[i].value;
                            if(tokens[i + 1].type == equals){
                                i += 2;
                                std::string attValue = tokens[i].value;
                                auto attribute = std::make_unique<node>(ATTRIBUTE_NODE, attName);
                                auto value = std::make_unique<node>(TEXT_NODE, attValue);
                                attribute->appendChild(std::move(value));
                                element->appendChild(std::move(attribute));
                            }
                        }
                        bool selfClosing = false;
                        if(i + 1 < tokens.size() && tokens[i + 1].type == slash){
                            selfClosing = true;
                            i++; 
                        }
                        if(i + 1 < tokens.size() && tokens[i + 1].type == gt){
                            i++;
                        }

                        node* elemPtr = stack.top()->appendChild(std::move(element));

                        if(!selfClosing){
                            stack.push(elemPtr);
                        }
                    }else if(t.type == slash){
                        if(i + 1 < tokens.size() && tokens[i + 1].type == identifier){
                            i++;
                        }
                        if(i + 1 < tokens.size() && tokens[i + 1].type == gt){
                            i++;
                        }
                        if(stack.size() > 1){
                            stack.pop();
                        }
                        continue;
                    }else if(t.type == string || t.type == identifier){
                        std::string text = t.value;
                        while(i + 1 < tokens.size() && (tokens[i + 1].type == identifier || tokens[i + 1].type == string)){
                            text += ' ' + tokens[++i].value;
                        }
                        stack.top()->appendChild(std::make_unique<node>(TEXT_NODE, text));
                    }
                    
                }
            }
   };
}