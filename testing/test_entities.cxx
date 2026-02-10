#include "../include/miniXML/document.hpp"
#include <iostream>

using namespace miniXML;
using namespace miniXML::details;

int main(){
    document d("file.xml");
    auto& root = d.rootNode();

    auto note = root.findChild("note");
    auto newNode = std::make_unique<node>(node_type::ELEMENT_NODE, "new");
    auto text = std::make_unique<node>(node_type::TEXT_NODE, "2 < 3");
    newNode->appendChild(std::move(text));
    note->appendChild(std::move(newNode));

    std::cout << root.toString() << std::endl;
    d.writeToFile("new.xml");
    return 0;
}