#pragma once
#include "node.hpp"
#include <fstream>
#include <sstream>

// The implementation of the document class as a top level controller of the tree.
namespace miniXML{
    class document{
        public:
            //constructor used for reading from a file
            document(const std::string& filepath) : root(details::node_type::DOCUMENT_NODE, "") {
                std::ifstream f(filepath);
                if(!f){
                    throw std::runtime_error("Failed to open file");
                }
                std::stringstream buffer;
                
                buffer << f.rdbuf();

                content = buffer.str();
                tokenize();
                buildTree();
            }
            //constructor used for generating a document
            document() : root(details::node_type::DOCUMENT_NODE, ""){}

            [[nodiscard]] node& rootNode() noexcept {
                return root;
            }
            [[nodiscard]] const node& rootNode() const noexcept {
                return root;
            }
            void writeToFile(const std::string& filepath, int depth = 0) const {
                std::ofstream file(filepath);
                for(const auto& c : root.getChildren()){
                    writeNode(*c, file, depth);    
                }
            }
            void parseFromString(std::string_view xmlContent){
                content.assign(xmlContent);
                tokens.clear();
                root.clearChildren();
                tokenize();
                buildTree();
            }
        private:
            node root;
            std::string content;
            std::vector<details::token> tokens;

            //defined in parser.hpp
            void tokenize();
            void buildTree();
            std::unique_ptr<node> parseElement(int& i);
            void writeNode(const node& n, std::ostream& file, int depth = 0) const;
   };
}