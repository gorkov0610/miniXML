#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include <optional>
#include <algorithm>
#include <fstream>
#include "types.hpp"


// Main tree implementation.
namespace miniXML{  
    class node{
        public:
            virtual ~node() = default;
            details::node_type getType() const noexcept {
                return type;
            }
            node* getParent() noexcept {
                return parent;
            }
            const node* getParent() const noexcept {
                return parent;
            }
            
            
            //setters
            virtual void setParent(node* n) noexcept {
                parent = n;
            }
            void setType(const details::node_type n) noexcept {
                type = n;
            }

            [[nodiscard]] virtual std::string toString(int depth = 0) const = 0;    
            virtual void write(std::ostream& file, int depth = 0) const = 0;
            
        protected:
            node(details::node_type type) : type(type){}
        private:
            details::node_type type;
            node* parent = nullptr;
    };
    class valueNode : virtual public node{
        public:
        valueNode(const details::node_type t) : node(t) {}
        virtual const std::string& getValue() const noexcept = 0;
        virtual void setValue(const std::string&) noexcept = 0;
        virtual void write(std::ostream& file, int depth = 0) const = 0;
    };
    class containerNode : virtual public node{
        public:
            containerNode(const details::node_type t) : node(t) {}
            virtual node* appendChild(std::unique_ptr<node>) = 0;
            virtual const std::vector<std::unique_ptr<node>>& getChildren() const = 0;
            virtual std::vector<std::unique_ptr<node>>& getChildren() = 0;
            virtual void write(std::ostream& file, int depth = 0) const = 0; 
    };
    class commentNode : public valueNode{
        public:
            commentNode(const std::string& v) : node(details::node_type::COMMENT_NODE), valueNode(details::node_type::COMMENT_NODE), value(v) {}

            const std::string& getValue() const noexcept override{
                return value;
            }
            void setValue(const std::string& value) noexcept override {
                this->value = value;
            }
            void write(std::ostream& file, int depth = 0) const override{
                file << std::string(depth * 2, ' ')  << "<!--" << value << "-->\n";
            }
            std::string toString(int depth=0) const override{
                std::string xml(depth * 2, ' ');
                return xml + "<!--" + getValue() + "-->\n";
            }
        private:
            std::string value;
    };
    class processingInstructionNode : public valueNode{
        public:
            processingInstructionNode(const std::string& v) : node(details::node_type::PROCESSING_INSTRUCTION_NODE), valueNode(details::node_type::PROCESSING_INSTRUCTION_NODE), value(v) {}

            const std::string& getValue() const noexcept override{
                return value;
            }

            void setValue(const std::string& value) noexcept override{
                this->value = value;
            }

            void write(std::ostream& file, int depth = 0) const override{
                file << std::string(depth * 2, ' ') << "<?" << value << "?>\n";
            }

            std::string toString(int depth=0) const override{
                std::string xml(depth * 2, ' ');

                return xml += "<?" + getValue() + "?>\n";
            }
        private:
            std::string value;
    };

    class elementNode : public valueNode, public containerNode{
        public:
            elementNode(const std::string& name) : node(details::node_type::ELEMENT_NODE), valueNode(details::node_type::ELEMENT_NODE), containerNode(details::node_type::ELEMENT_NODE), qualifiedName(name) {
                auto colon = name.find(':');

                if(colon == std::string::npos){
                    localName = qualifiedName;
                    prefix = "";
                }else{
                    prefix = name.substr(0, colon);
                    localName = name.substr(colon + 1);
                }
            }
            
            void setValue(const std::string& name) noexcept override {
                qualifiedName = name;
                auto colon = name.find(':');

                if(colon == std::string::npos){
                    localName = qualifiedName;
                    prefix = "";
                }else{
                    prefix = name.substr(0, colon);
                    localName = name.substr(colon + 1);
                }
            }
            const miniXML::details::namespaceInfo* getNamespace() const noexcept {
                return ns;
            }
            const std::string& getPrefix() const noexcept {
                return prefix;
            }
            std::string& getPrefix() noexcept {
                return prefix;
            }
            const std::vector<miniXML::details::attribute>& getAttributes() const{
                return attributes;
            }
            std::vector<miniXML::details::attribute>& getAttributes(){
                return attributes;
            }
            const std::string& getValue() const noexcept override{
                return qualifiedName;
            }
            [[nodiscard]] const std::vector<std::unique_ptr<node>>& getChildren() const override{
                return children;
            }
            [[nodiscard]] std::vector<std::unique_ptr<node>>& getChildren() override{
                return children;
            }
            [[nodiscard]] miniXML::details::attribute* getAttributeNS(std::string_view key, std::string_view uri) noexcept {
                for(auto& a : attributes){
                    if(a.ns && a.ns->url == uri && a.localName == key){
                        return &a;
                    }
                }
                return nullptr;
            }
            [[nodiscard]] miniXML::details::attribute* getAttribute(std::string_view key) noexcept {
                for(auto& a : attributes){
                    if(!a.ns && a.localName == key){
                        return &a;
                    }
                }
                return nullptr;
            }
            template <typename T>
            [[nodiscard]] T* findChild(const std::string& name){
                static_assert(std::is_base_of_v<valueNode, T>, "T must derive from valueNode");
                for(auto& c : children){
                    if(auto* casted = dynamic_cast<T*>(c.get())){
                        if(casted->getValue() == name){
                            return casted;
                        }
                    }
                }
                return nullptr;
            }
            template <typename T>
            [[nodiscard]] const T* findChild(const std::string& name) const {
                static_assert(std::is_base_of_v<valueNode, T>, "T must derive from valueNode");
                for(auto& c : children){
                    if(auto* casted = dynamic_cast<T*>(c.get())){
                        if(casted->getValue() == name){
                            return casted;
                        }
                    }
                }
                return nullptr;
            }
            [[nodiscard]] std::vector<node*> findChildren(details::node_type t){
                std::vector<node*> results;
                
                for(const auto& c : children){
                    if(c->getType() == t){
                        results.push_back(c.get());
                    }
                }
                return results;
            }
            [[nodiscard]] std::vector<const node*> findChildren(details::node_type t) const {
                std::vector<const node*> results;
                
                for(const auto& c : children){
                    if(c->getType() == t){
                        results.push_back(c.get());
                    }
                }
                return results;
            }
            void clearChildren(){
                for(auto& c : children){
                    c->setParent(nullptr);
                }
                children.clear();
            }
            void clearAttributes(){
                attributes.clear();
            }
            [[nodiscard]] const miniXML::details::attribute* getAttribute(std::string_view key) const noexcept {
                for(const auto& a : attributes){
                    if(a.ns == nullptr && a.localName == key){
                        return &a;
                    }
                }
                return nullptr;
            }

            void appendAttribute(std::string key, std::string value){
                attributes.emplace_back(key, value);
            }
            node* appendChild(std::unique_ptr<node> n) override{
                if(!n){
                    return nullptr;
                }
                if(n->getParent() != nullptr){
                    return nullptr;
                }
                n->setParent(this);
                children.push_back(std::move(n));
                return children.back().get();
            }

            [[nodiscard]] bool deleteAttribute(std::string_view key){
                auto it = std::find_if(attributes.begin(), attributes.end(), [key](const miniXML::details::attribute& a){
                    return key == a.localName;
                });

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

                (*it)->setParent(nullptr);
                children.erase(it);
                return true;
            }

            void write(std::ostream& file, int depth = 0) const override{
                const std::string ind(depth * 2, ' ');
                file << ind << "<" << qualifiedName;
                for(const auto& a : attributes){
                    file << " " << a.qualifiedName << "=\"" << a.value << "\"";
                }

                if(children.empty()){
                    file << "/>\n";
                }else{
                    file << ">\n";
                    for(const auto& c : children){
                        c->write(file, depth + 1);
                    }
                }

                file << ind << "</" << qualifiedName << ">\n";
            }
            std::string toString(int depth = 0 ) const override{
                std::string xml(depth * 2, ' ');
                xml += "<" + getValue();    
                for(const auto& a : attributes){
                    xml += " " +  a.qualifiedName + "=\"" + a.value + "\""; 
                }
                if(children.empty()){
                    xml += "/>\n";
                }else{
                    xml += ">\n";
                    for(const auto& c : children){
                        xml += c->toString(depth + 1);
                    }
                    xml += std::string(depth * 2, ' ') +  "</" + getValue() + ">\n";
                }
                return xml;
            }
        private:
            std::string qualifiedName;
            std::string localName;
            std::string prefix;
            const details::namespaceInfo* ns = nullptr;
            
            std::vector<std::unique_ptr<node>> children;
            std::vector<details::attribute> attributes;
            std::unordered_map<std::string, std::shared_ptr<miniXML::details::namespaceInfo>> namespaces;
            void registerNamespace(std::string prefix, std::string uri){
                namespaces[prefix] = std::make_shared<details::namespaceInfo>(std::move(prefix), std::move(uri));
            }
            friend class document;
    };
    class textNode : public valueNode{
        public:
            textNode(const std::string& value) : node(details::node_type::TEXT_NODE), valueNode(details::node_type::TEXT_NODE), value(value) {}

            const std::string& getValue() const noexcept override{
                return value;
            }

            void setValue(const std::string& v) noexcept override{
                value = v;
            }

            std::string toString(int depth = 0 ) const override{ 
                return std::string(depth * 2, ' ') + value + '\n';
            }
            void write(std::ostream& file, int depth = 0) const override{
                const std::string ind(depth * 2 , ' ');
                file << ind << value << "\n";
            }
        private:
            std::string value;
    };
    class documentNode : public containerNode{
        public:
            documentNode() : node(details::node_type::DOCUMENT_NODE), containerNode(details::node_type::DOCUMENT_NODE){}

            [[nodiscard]] const std::vector<std::unique_ptr<node>>& getChildren() const override {
                return children;
            }
            [[nodiscard]] std::vector<std::unique_ptr<node>>& getChildren() override{
                return children;
            }
            node* appendChild(std::unique_ptr<node> n) override {
                if(!n){
                    return nullptr;
                }
                if(n->getParent() != nullptr){
                    return nullptr;
                }
                n->setParent(this);
                children.push_back(std::move(n));
                return children.back().get();
            }
            template <typename T>
            [[nodiscard]] T* findChild(const std::string& name){
                static_assert(std::is_base_of_v<valueNode, T>, "T must derive from valueNode");
                for(auto& c : children){
                    if(auto* casted = dynamic_cast<T*>(c.get())){
                        if(casted->getValue() == name){
                            return casted;
                        }
                    }
                }
                return nullptr;
            }
            template <typename T>
            [[nodiscard]] const T* findChild(const std::string& name) const {
                static_assert(std::is_base_of_v<valueNode, T>, "T must derive from valueNode");
                for(auto& c : children){
                    if(auto* casted = dynamic_cast<T*>(c.get())){
                        if(casted->getValue() == name){
                            return casted;
                        }
                    }
                }
                return nullptr;
            }
            [[nodiscard]] std::vector<node*> findChildren(details::node_type t){
                std::vector<node*> results;
                
                for(const auto& c : children){
                    if(c->getType() == t){
                        results.push_back(c.get());
                    }
                }
                return results;
            }
            [[nodiscard]] std::vector<const node*> findChildren(details::node_type t) const {
                std::vector<const node*> results;
                
                for(const auto& c : children){
                    if(c->getType() == t){
                        results.push_back(c.get());
                    }
                }
                return results;
            }
            void clearChildren(){
                for(auto& c : children){
                    c->setParent(nullptr);
                }
                children.clear();
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

                (*it)->setParent(nullptr);
                children.erase(it);
                return true;
            }
            std::string toString(int depth=0) const override{
                std::string xml;
                for(const auto& c : children){
                    xml += c->toString(depth);
                }
            return xml;
            }
            void write(std::ostream& file, int depth = 0) const override{
                for(const auto& c : children){
                    c->write(file, depth);
                }
            }
        private:
            std::vector<std::unique_ptr<node>> children;
            friend class document;
    };
};