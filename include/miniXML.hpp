#pragma once
#include <vector>
#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace miniXML{
    enum token_type{
        lt, gt, slash, equals,
        identifier, 
        string,
        question,
        dash,
        exclamation
    };
    enum node_type{
        ELEMENT_NODE = 1,
        ATTRIBUTE_NODE,
        TEXT_NODE,
        DOCUMENT_NODE,
        COMMENT_NODE,
        PROCESSING_INSTRUCTION_NODE
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
            node* getParrent(){
                return parrent;
            }
            void setType(const node_type n){
                type = n;
            }
            void setValue(const std::string& n){
                value = n;
            }

            node* appendChild(std::unique_ptr<node> n){
                n->parrent = this;
                children.push_back(std::move(n));
                return children.back().get();
            }
            bool deleteChild(const std::string& name){
                auto it = std::find_if(children.begin(), children.end(), [name](const std::unique_ptr<node>& n){
                    return name == n->value;
                });
                if(it != children.end()){
                    (*it)->parrent = nullptr;
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
                    (*it)->parrent = nullptr;
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
                    (*it)->parrent = nullptr;
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
            node* parrent = nullptr;
    };
    class document{
        public:
            document(const std::string& filepath) : root(DOCUMENT_NODE, "") {
                std::ifstream f(filepath);
                if(!f){
                    throw std::runtime_error("Failed to open file");
                }
                std::stringstream buffer;
                
                buffer << f.rdbuf();

                file = buffer.str();
                tokenize();
                buildTree();
            }
            document() : root(DOCUMENT_NODE, ""){}

            node& rootNode(){
                return root;
            }
            const node& rootNode() const{
                return root;
            }
            void writeToFile(const std::string& filepath, int depth = 0){
                std::ofstream file(filepath);
                for(const auto& c : root.getChildren()){
                    writeNode(*c, file, depth);    
                }
            }

            void parseFromString(std::string& xmlContent){
                file = xmlContent;
                tokenize();
                buildTree();
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
                        case '?':
                            tokens.push_back({question, "?"});
                            i++;
                            break;
                        case '-':
                            tokens.push_back({dash, "-"});
                            i++;
                            break;
                        case '!':
                            tokens.push_back({exclamation, "!"});
                            i++;
                            break;
                        default:
                            if(std::isspace(file[i])){
                                i++;
                            }else{
                                while(i < file.size() && !std::isspace(file[i]) && file[i] != '<' && file[i] != '>' && file[i] != '=' && file[i] != '/' && file[i] != '?' && file[i] != '-'){
                                    s += file[i++];
                                }
                                tokens.push_back({identifier, s});
                            }
                            break;

                    }
                }
            }

            void buildTree(){
                auto i{0};
                while(i < tokens.size()){
                    if(tokens[i].type == lt && tokens[i + 1].type == identifier){
                        auto child = parseElement(i);
                        if(child){
                            root.appendChild(std::move(child));
                        }
                    }else if(i + 1 < tokens.size() && tokens[i].type == lt && tokens[i + 1].type == exclamation){
                        i += 4;
                        std::string comment;
                        while(i + 1 < tokens.size() && tokens[i].type != dash){
                            comment += tokens[i].value + " ";
                            ++i;
                        }
                        auto commentNode = std::make_unique<node>(COMMENT_NODE, comment);
                        root.appendChild(std::move(commentNode));
                    }else if(i + 1 < tokens.size() && tokens[i].type == lt && tokens[i + 1].type == question){
                        i += 2;
                        std::string pi;
                        while(i + 1 < tokens.size() && tokens[i].type != question){
                            if(tokens[i].type == identifier && tokens[i + 1].type == equals){
                                pi += " " + tokens[i].value;
                            }else if(tokens[i].type == equals){
                                pi += tokens[i].value + "\"";
                            }else if(tokens[i].type == string){
                                pi += tokens[i].value + "\"";
                            }else{
                                pi += tokens[i].value;
                            }
                            i++;
                        }
                        auto piNode = std::make_unique<node>(PROCESSING_INSTRUCTION_NODE, pi);
                        root.appendChild(std::move(piNode));
                    }else{
                        i++;
                    }
                }
            }
            std::unique_ptr<node> parseElement(int& i){
                if(i + 1 >= tokens.size() && tokens[i].type != lt){
                    return nullptr;
                }
                ++i;

                std::string name = tokens[i++].value;
                auto element = std::make_unique<node>(ELEMENT_NODE, name);
                while(i < tokens.size() && tokens[i].type == identifier){
                    std::string attName = tokens[i++].value;
                    if(i < tokens.size() && tokens[i].type == equals){
                        ++i;
                        std::string attValue = tokens[i++].value;
                        
                        auto attribute = std::make_unique<node>(ATTRIBUTE_NODE, attName);
                        attribute->appendChild(std::make_unique<node>(TEXT_NODE, attValue));
                        element->appendChild(std::move(attribute));
                    }
                }
                if(i + 1 < tokens.size() && tokens[i].type == slash && tokens[i + 1].type == gt){
                    i += 2;
                    return element;
                }
                if(i < tokens.size() && tokens[i].type == gt){
                    i++;
                }
                if(i + 1 < tokens.size() && tokens[i].type == lt && tokens[i + 1].type == exclamation){
                    i += 4;
                    std::string comment;
                    while(i + 1 < tokens.size() && tokens[i].type != dash){
                        comment += tokens[i].value + " ";
                        ++i;
                    }
                    auto commentNode = std::make_unique<node>(COMMENT_NODE, comment);
                    element->appendChild(std::move(commentNode));
                }

                while(i + 1 < tokens.size() && !(tokens[i].type == lt && tokens[i + 1].type == slash)){
                    if(tokens[i].type == lt && (tokens[i + 1].type == identifier || tokens[i + 1].type == string)){
                        element->appendChild(parseElement(i));
                    }else if(tokens[i].type == identifier && tokens[i + 1].type != dash){
                        std::string text;
                        while(i < tokens.size() && (tokens[i].type == string || tokens[i].type == identifier)){
                            text += tokens[i++].value + ' ';
                        }
                        if(!text.empty()){
                            text.pop_back();
                        }
                        element->appendChild(std::make_unique<node>(TEXT_NODE, text));
                    }else if(tokens[i].type == lt && tokens[i + 1].type == exclamation){
                        i += 4;
                        std::string comment;
                        while(i + 1 < tokens.size() && tokens[i].type != dash){
                            comment += tokens[i].value + " ";
                            ++i;
                        }
                        auto commentNode = std::make_unique<node>(COMMENT_NODE, comment);
                        element->appendChild(std::move(commentNode));
                    }else{
                        i++;
                    }
                }
                
                if(i + 2 < tokens.size() && tokens[i].type == lt && tokens[i + 1].type == slash && tokens[i + 2].type == identifier){
                    i += 3;
                    if(i < tokens.size() && tokens[i].type == gt){
                        ++i;
                    }
                }
                return element;
            }
            void writeNode(const node& n, std::ofstream& file, int depth = 0){
                const std::string ind(depth * 2, ' ');
                switch (n.getType()){
                case ELEMENT_NODE:{
                    file << ind << "<" << n.getValue();
                    
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
    
                    if(n.findChildren(TEXT_NODE).empty() && n.findChildren(ELEMENT_NODE).empty()){
                        file << "/>\n";
                        return;
                    }
                    
                    file << ">\n";

                    for(const auto& c : n.getChildren()){
                        if(c->getType() != ATTRIBUTE_NODE){
                            writeNode(*c, file, depth + 1);
                        }
                    }

                    file << ind << "</" + n.getValue() + ">\n";
                    break;
                }
                case COMMENT_NODE:{
                    file << ind << "<!--" << n.getValue() << "-->" << std::endl;
                    break;
                }
                case TEXT_NODE:{
                    file << ind << n.getValue() << std::endl;
                    break;
                }
                case PROCESSING_INSTRUCTION_NODE:{
                    file << ind << "<?" << n.getValue() << "?>\n";
                }
                default:
                    break;
                }
            }
   };
}