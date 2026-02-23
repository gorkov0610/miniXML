#pragma once
#include "node.hpp"
#include <fstream>
#include <sstream>

// The implementation of the document class as a top level controller of the tree.
namespace miniXML{
    class document{
        public:
            //constructor used for reading from a file
            document(const std::string& filepath) {
                std::ifstream f(filepath);
                if(!f){
                    throw std::runtime_error("Failed to open file: " + filepath);
                }
                std::stringstream buffer;
                
                buffer << f.rdbuf();

                content = buffer.str();
                tokenize();
                buildTree();
                resolveAllNamespaces();
            }
            //constructor used for generating a document
            document() = default;

            [[nodiscard]] documentNode& rootNode() noexcept {
                return root;
            }
            [[nodiscard]] const documentNode& rootNode() const noexcept {
                return root;
            }
            void writeToFile(const std::string& filepath, int depth = 0) const {
                if(depth < 0){
                    depth = 0;
                }
                std::ofstream file(filepath);
                if(!file){
                    throw std::runtime_error("Unable to open: " + filepath);
                }
                root.write(file, depth);
            }
            void parseFromString(std::string_view xmlContent){
                content.assign(xmlContent);
                tokens.clear();
                root.clearChildren();
                tokenize();
                buildTree();
                resolveAllNamespaces();
            }
            
        private:
            documentNode root;
            std::string content;
            std::vector<details::token> tokens;

            //defined in parser.hpp
            void tokenize();
            void buildTree();
            std::unique_ptr<node> parseElement(size_t& i);

            void resolveElementNamespace(elementNode* n);
            void resolveAttributeNamespace(elementNode* n);
            void resolveAllNamespaces();
   };
}

#include "parser.hpp"